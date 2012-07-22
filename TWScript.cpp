
#include "TWScript.h"
#include "ScriptModule.h"

#include <lg/interface.h>
#include <lg/scrmanagers.h>
#include <lg/scrservices.h>
#include <lg/objects.h>
#include <lg/links.h>
#include <lg/properties.h>
#include <lg/propdefs.h>

#include "ScriptLib.h"
#include "utils.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <list>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <memory>

/* =============================================================================
 *  TWTweqSmooth Impmementation
 */

const char *cScr_TWTweqSmooth::axis_names[] = {
    "x rate-low-high",
    "y rate-low-high",
    "z rate-low-high"
};

const char *cScr_TWTweqSmooth::joint_names[] = {
    "    rate-low-high",
    "    rate-low-high2",
    "    rate-low-high3",
    "    rate-low-high4",
    "    rate-low-high5",
    "    rate-low-high6"
};

/* -----------------------------------------------------------------------------
 *  TWScript functions
 */

cAnsiStr TWScript::get_object_namestr(object obj_id)
{
    cAnsiStr str = "0";
    if(obj_id) {
        SInterface<IObjectSystem> pOS(g_pScriptManager);
        const char* pszName = pOS->GetName(obj_id);

        if (pszName) {
            str.FmtStr("%s (%d)", pszName, int(obj_id));

        } else {
            SInterface<ITraitManager> pTM(g_pScriptManager);
            object iArc = pTM->GetArchetype(obj_id);
            pszName = pOS->GetName(iArc);

            if (pszName)
                str.FmtStr("A %s (%d)", pszName, int(obj_id));
            else
                str.FmtStr("%d", int(obj_id));
        }
    }
    return str;
}


long TWScript::get_qvar_value(const char *qvar, long def_val)
{
    SService<IQuestSrv> pQS(g_pScriptManager);
    if(pQS -> Exists(qvar))
       return pQS -> Get(qvar);

    return def_val;
}


float TWScript::get_qvar_value(const char *qvar, float def_val)
{
    // Blegh, copy. But we may need to tinker with it.
    char *tmpvar = (char *)g_pMalloc -> Alloc(strlen(qvar) + 1);
    if(!tmpvar) return def_val;
    strcpy(tmpvar, qvar);

    // Check whether the user has included

}


bool TWScript::radius_search(char *dest, float *radius, bool *lessthan, char **archetype)
{
    char  mode   = 0;
    char *search = dest;

    // Search the string for a < or >, if found, record it and the start of the archetype
    while(*search) {
        if(*search == '<' || *search == '>') {
            mode = *search;
            *lessthan = (mode == '<');
            *archetype = search + 1;
            break;
        }
        ++search;
    }

    // If the mode hasn't been found, or there's no archetype set, it's not a radius search
    if(!mode || !*archetype) return false;

    // It's a radius search, so try to parse the radius
    char *end;
    *radius = strtof(dest, &end);

    // If the value didn't parse, or parsing stopped before the < or >, give up (the latter
    // is actually probably 'safe', but meh)
    if(end == dest || *end != mode) return false;

    // Okay, this should be a radius search!
    return true;
}


void TWScript::archetype_search(std::vector<object> *matches, char *archetype, bool do_full, bool do_radius, object from_obj, float radius, bool lessthan)
{
    SInterface<IObjectSystem> pOS(g_pScriptManager);
	SService<IObjectSrv> pOSrv(g_pScriptManager);
    SInterface<ITraitManager> pTM(g_pScriptManager);

    // These are only needed when doing radius searches
    cScrVec from_pos, to_pos;
    float   distance;
    if(do_radius) pOSrv -> Position(from_pos, from_obj);

    // Find the archetype named if possible
    object arch = pOS -> GetObjectNamed(archetype);
    if(int(arch) <= 0) {

        // Build the query flags
        ulong flags = kTraitQueryChildren;
        if(do_full) flags |= kTraitQueryFull; // If dofull is on, query direct and indirect descendants

        // Ask for the list of matching objects
        SInterface<IObjectQuery> query = pTM -> Query(arch, flags);
        if(query) {

            // Process each object, adding it to the match list if it's concrete.
            for(; !query -> Done(); query -> Next()) {
                object obj = query -> Object();
                if(int(obj) > 0) {

                    // Object is concrete, do we need to check it for distance?
                    if(do_radius) {
                        // Get the provisionally matched object's position, and work out how far it
                        // is from the 'from' object.
                        pOSrv -> Position(to_pos, obj);
                        distance = (float)from_pos.Distance(to_pos);

                        // If the distance check passes, store the object.
                        if((lessthan && (distance < radius)) || (!lessthan && (distance > radius))) {
                            matches -> push_back(obj);
                        }

                    // No radius check needed, add straight to the list
                    } else {
                        matches -> push_back(obj);
                    }
                }
            }
        }
    }
}


std::vector<object>* TWScript::get_target_objects(char *dest, sScrMsg *pMsg)
{
    std::vector<object>* matches = new std::vector<object>;

    float radius;
    bool  lessthan;
    char *archname;

    // Simple dest/source selection.
    if(!_stricmp(dest, "[me]")) {
        matches -> push_back(pMsg -> to);

    } else if(!_stricmp(dest, "[source]")) {
        matches -> push_back(pMsg -> from);

    // Archetype search, direct concrete and indirect concrete
    } else if(*dest == '*' || *dest == '@') {
        archetype_search(matches, &dest[1], *dest == '@');

    // Radius archetype search
    } else if(radius_search(dest, &radius, &lessthan, &archname)) {
        char *realname = archname;
        // Jump filter controls if needed...
        if(*archname == '*' || *archname == '@') ++realname;

        // Default behaviour for radius search is to get all decendants unless * is specified.
        archetype_search(matches, realname, *archname != '*', true, pMsg -> to, radius, lessthan);

    // Named destination object
    } else {
        SInterface<IObjectSystem> pOS(g_pScriptManager);

        object obj = pOS -> GetObjectNamed(dest);
        if(obj) matches -> push_back(obj);
    }

    return matches;
}


/* -----------------------------------------------------------------------------
 *  Timer related
 */

void cScr_TWTweqSmooth::start_timer()
{
    timer = SetTimedMessage("TWRateUpdate", timer_rate, kSTM_OneShot, "TWScripts");
}


void cScr_TWTweqSmooth::clear_timer()
{
    if(timer) {
        KillTimedMessage(timer);
        timer = NULL;
    }

}


/* -----------------------------------------------------------------------------
 *  Rate and bounds utility functions
 */

float cScr_TWTweqSmooth::get_rotate_rate(RotAxis axis)
{
    SService<IPropertySrv> pPS(g_pScriptManager);
    cMultiParm prop;

    pPS -> Get(prop, ObjId(), "CfgTweqRotate", axis_names[axis]);

    // The float cast here is probably redundant, but meh.
    return (float)(static_cast<const mxs_vector*>(prop) -> x);
}


bool cScr_TWTweqSmooth::valid_rotate_bounds(RotAxis axis)
{
    SService<IPropertySrv> pPS(g_pScriptManager);
    cMultiParm prop;

    pPS -> Get(prop, ObjId(), "CfgTweqRotate", axis_names[axis]);

    float low  = static_cast<const mxs_vector*>(prop) -> y;
    float high = static_cast<const mxs_vector*>(prop) -> z;

    return(low >= 0.0f && high > low);
}


float cScr_TWTweqSmooth::get_joint_rate(int joint)
{
    SService<IPropertySrv> pPS(g_pScriptManager);
    cMultiParm prop;

    pPS -> Get(prop, ObjId(), "CfgTweqJoints", joint_names[joint]);

    // The float cast here is probably redundant, but meh.
    return (float)(static_cast<const mxs_vector*>(prop) -> x);
}


float cScr_TWTweqSmooth::get_rate_param(const char *design_note, const char *cfgname, float default_value, float minimum)
{
    float value = GetParamFloat(design_note, cfgname, default_value);

    if(value < minimum) value = minimum;

    return value;
}


/* -----------------------------------------------------------------------------
 *  Initialisation
 */

int cScr_TWTweqSmooth::init_rotate_onoffctrl(char *axes)
{
    // Handle being passed nothing, or an empty string
    if(!axes || !*axes) return 0;

    bool all  = !strcasecmp(axes, "all");
    int count = 0;

    // First initialise the axes either to all on (if all is set), or
    // all off if it is not.
    for(int axis = 0; axis < RotAxisMax; ++axis) {
        axis_smooth[axis] = all;
        if(all) ++count;
    }

    // If all is not set, check which axes have been selected for smoothing, if any
    if(!all) {

        // Check the whole axes string for names, this will ignore any characters
        // other than upper or lower case x, y, or z.
        while(*axes) {

            // Realistically, this could probably update *axes, but the temp won't hurt.
            char current = tolower(*axes);
            if(current >= 'x' && current <= 'z') {

                // Only change the axes, and count the change, the first time.
                if(!axis_smooth[current - 'x']) {
                    axis_smooth[current - 'x'] = true;
                    ++count;
                }
            }
            ++axes;
        }
    }

    return count;
}


void cScr_TWTweqSmooth::init_rotate(char *design_note)
{
    SService<IPropertySrv> pPS(g_pScriptManager);
    int      set_count  = 0;
    bool     can_smooth = false;
    cAnsiStr obj_name   = get_object_namestr(ObjId());

    // Find out whether any axes have been selected for smoothing
    char *axis = GetParamString(design_note, "TWTweqSmoothRotate", "all");
    if(axis) {
        set_count = init_rotate_onoffctrl(axis);
        g_pMalloc -> Free(axis);
    }

    // If the object has a TweqRotate configuration, check that the AnimC flags are okay
    if(pPS -> Possessed(ObjId(), "CfgTweqRotate")) {
        cMultiParm flags;
        pPS -> Get(flags, ObjId(), "CfgTweqRotate", "AnimC");

        // If the flags are not compatible, print a warning in the monolog
        can_smooth = compatible_animc(static_cast<int>(flags));
        if(!can_smooth) {

            if(warnings)
                DebugPrintf("WARNING[TWTweqSmooth]: %s has incompatible AnimC flags. Unable to smooth rotation.", static_cast<const char *>(obj_name));
        }
    }

    // Does the object have a rotate tweq, configuration, and the matching rotate state?
    // This also allows the user to suppress smoothing on an object's rotate if needed.
    do_tweq_rotate = (can_smooth &&
                      pPS -> Possessed(ObjId(), "CfgTweqRotate") &&
                      pPS -> Possessed(ObjId(), "StTweqRotate") &&
                      set_count);

    // If the rotate is being smoothed, obtain maximum and minimum rates for each axis.
    // If the user hasn't specified them, the global defaults are used instead.
    if(do_tweq_rotate) {
        rotate_rates[RotXAxis][MinRate] = get_rate_param(design_note, "TWTweqSmoothRotateXMin", min_rate);
        rotate_rates[RotYAxis][MinRate] = get_rate_param(design_note, "TWTweqSmoothRotateYMin", min_rate);
        rotate_rates[RotZAxis][MinRate] = get_rate_param(design_note, "TWTweqSmoothRotateZMin", min_rate);

        // Axis maximum rates are taken from the Tweq -> Rotate coniguration for each axis.
        for(int axis = 0; axis < RotAxisMax; ++axis) {
            if(axis_smooth[axis]) {
                rotate_rates[axis][MaxRate] = get_rotate_rate((RotAxis)axis);

                // If the axis max rate is not set or too low, mark it as not smoothed.
                if(rotate_rates[axis][MaxRate] <= rotate_rates[axis][MinRate]) {
                    axis_smooth[axis] = false;
                    --set_count;

                    if(warnings)
                        DebugPrintf("WARNING[TWTweqSmooth]: %s %s has rate set to the min rate or less, disabling smoothing on this axis.", static_cast<const char *>(obj_name), axis_names[axis]);

                // Similarly, if the low/high bounds on the rotation are not good, disable smoothing
                } else if(!valid_rotate_bounds((RotAxis)axis)) {
                    axis_smooth[axis] = false;
                    --set_count;

                    if(warnings)
                        DebugPrintf("WARNING[TWTweqSmooth]: %s %s has unsupported bounds, disabling smoothing on this axis.", static_cast<const char *>(obj_name), axis_names[axis]);
                }
            }
        }

        // If all the axes have been disabled, disable all smoothing
        if(!set_count) {
            do_tweq_rotate = false;

            if(warnings)
                DebugPrintf("WARNING[TWTweqSmooth]: %s has no smoothable axes after rate checks. Rotation smoothing disabled.", static_cast<const char *>(obj_name));
        }
    } else if(warnings) {
        DebugPrintf("NOTICE[TWTweqSmooth]: %s rotation smoothing disabled.", static_cast<const char *>(obj_name));
    }
}


int cScr_TWTweqSmooth::init_joints_onoffctrl(char *joints)
{
    // Handle being passed nothing, or an empty string
    if(!joints || !*joints) return 0;

    bool all  = !strcasecmp(joints, "all");
    int count = 0;

    // First initialise the joints either to all on (if all is set), or
    // all off if it is not.
    for(int joint = 0; joint < JointCount; ++joint) {
        joint_smooth[joint] = all;
        if(all) ++count;
    }

    // If all is not set, individual joints have been selected for smoothing.
    if(!all) {
        char *cont = joints;

        // This loops over the joints string, parsing out comma separated joint
        // numbers and enabling the joints encountered.
        while(*cont) {
            int joint = strtol(joints, &cont, 10);
            if(cont == joints) break;    // Halt if parse failed
            if(*cont) joints = cont + 1; // Keep going if not at the end of string

            // If the joint specified by the user is in range, enable it
            if(joint > 0 && joint <= JointCount) {
                // Note -1 here. The UI has the joints 1-indexed, this is 0 indexed!
                joint_smooth[joint - 1] = true;
                ++count;
            }
        }
    }

    // Now go back and check that the joints can actually be smoothed. Easier to
    // do it here after the fact that while checking the string above...
    SService<IPropertySrv> pPS(g_pScriptManager);
    cAnsiStr joint_buffer;

    // Note that, if the object is missing CfgTweqJoints, it's no big problem
    // that we're not going to be able to check the joints - the check in
    // init_joints() will stop *all* smoothing actions there if it doesn't have one.
    if(pPS -> Possessed(ObjId(), "CfgTweqJoints")) {
        // Get the object name, in case it is needed in the loop
        cAnsiStr obj_name = get_object_namestr(ObjId());

        cMultiParm flags;
        for(int joint = 0; joint < JointCount; ++joint) {
            // Only bother checking the config if the joint is selected for smoothing
            if(joint_smooth[joint]) {
                // What's the AnimC we need for this joint?
                joint_buffer.FmtStr("Joint%dAnimC", joint + 1);

                // Get the flags, and check they are okay.
                pPS -> Get(flags, ObjId(), "CfgTweqJoints", static_cast<const char *>(joint_buffer));

                // Disable joints with incompatible animc
                if(!compatible_animc(static_cast<int>(flags))) {
                    joint_smooth[joint] = false;
                    --count;

                    if(warnings)
                        DebugPrintf("NOTICE[TWTweqSmooth]: %s has incompatible AnimC flags on joint %d. Unable to smooth joint %d rotation.", static_cast<const char *>(obj_name), joint + 1, joint + 1);
                }
            }
        }
    }

    return count;
}


void cScr_TWTweqSmooth::init_joints(char *design_note)
{
    SService<IPropertySrv> pPS(g_pScriptManager);
    int set_count = 0;

    // Find out how many joints have been selected for smoothing, and pass checks on their AnimC
    char *joints = GetParamString(design_note, "TWTweqSmoothJoints", "all");
    if(joints) {
        set_count = init_joints_onoffctrl(joints);
        g_pMalloc -> Free(joints);
    }

    // As near as I can tell, NoLimit and Wrap on the CfgTweqJoints main config (as opposed to on
    // individual joints) does bugger all, and can be ignored here unlike in init_rotate().

    // Only smooth joints if there are joints to smooth, and one or more are set for smoothing
    do_tweq_joints = (pPS -> Possessed(ObjId(), "CfgTweqJoints") &&
                      pPS -> Possessed(ObjId(), "StTweqJoints") &&
                      set_count);

    // If joint smoothing is enabled, the minimum and maximum rates for each joint
    // need to be set.
    if(do_tweq_joints) {
        cAnsiStr cfg_buffer;
        cAnsiStr obj_name = get_object_namestr(ObjId());

        for(int joint = 0; joint < JointCount; ++joint) {
            // Only bother setting the rates if the joint is selected for smoothing
            if(joint_smooth[joint]) {
                // Get the rates (the max comes out of the joint config)
                cfg_buffer.FmtStr("TWTweqSmoothJoint%1dMin", joint + 1);
                joint_rates[joint][MinRate] = get_rate_param(design_note, static_cast<const char *>(cfg_buffer), min_rate);
                joint_rates[joint][MaxRate] = get_joint_rate(joint);

                // Check it is smoothable.


                DebugPrintf("%s joint %d has rate %f", static_cast<const char *>(obj_name), joint, joint_rates[joint][MaxRate]);
            }
        }
    }
}


void cScr_TWTweqSmooth::init()
{
    // Ensure that the timer can't fire while re-initialising
    clear_timer();

    // Fetch the contents of the object's design note
    char *design_note = GetObjectParams(ObjId());

    if(design_note) {
        // Should warnings be displayed in the monolog?
        warnings = GetParamBool(design_note, "TWTweqSmoothWarn", true);

        // How frequently should the timer update the tweq rate?
        timer_rate = GetParamInt(design_note, "TWTweqSmoothTimerRate", 250);

        // Has the editor specified defaults for the min and max rates?
        min_rate = get_rate_param(design_note, "TWTweqSmoothMinRate", 0.1f);
        max_rate = get_rate_param(design_note, "TWTweqSmoothMaxRate", 10.0f, min_rate); // note: force max >= min.

        // Now we need to determine whether the object has a rotate tweq, and joint
        // tweq, or both, and set up the smoothing facility accordingly.
        init_rotate(design_note);
        init_joints(design_note);

        // If either rotate or joints are going to be smoothed, start the timer to do it.
        if(do_tweq_rotate || do_tweq_joints) {
            start_timer();

            // Otherwise potentially bitch at the user.
        } else if(warnings) {
            cAnsiStr obj_name = get_object_namestr(ObjId());
            DebugPrintf("WARNING[TWTweqSmooth]: %s has TweqRotate and TweqJoints smoothing disabled. Why am I on this object?", static_cast<const char *>(obj_name));
        }

        // No longer need the design note
        g_pMalloc->Free(design_note);
    }
}


/* -----------------------------------------------------------------------------
 *  Smoothing implementations
 */

void cScr_TWTweqSmooth::set_axis_rate(const char *propname, RotAxis axis)
{
    cMultiParm prop;
    float rate, low, high;

/*    SService<IPropertySrv> pPS(g_pScriptManager);

    pPS -> Get(prop, ObjId(), "CfgTweqRotate", propname);
    low  = static_cast<const mxs_vector*>(prop) -> y; // low
    high = static_cast<const mxs_vector*>(prop) -> z; // high

    // If either the low or high are non-zero, update the rate...
    if((low || high) && (low != high)) {
        // Where is the object facing?
        cScrVec facing;
        SService<IObjectSrv> pOS(g_pScriptManager);

        // Fetch the direction in degrees
        pOS -> Facing(facing, ObjId());

        float current;
        switch(axis) {
            case RotXAxis: current = facing.x; break;
            case RotYAxis: current = facing.y; break;
            case RotZAxis: current = facing.z; break;
            case RotAxisMax:
            default: current = low;
        }

        // Rate is a function of the current angle within the range
        rate = max_rate * sin(((current - low) * M_PI) / (high - low));

            //SService<IDebugScrSrv> pDSS(g_pScriptManager);
            //cAnsiStr outstr;
            //outstr.FmtStr(999, "EaseRotate: min = %0.2f; max = %0.2f; c = %0.2f; rate = %0.2f", low, high, current, rate);
            //pDSS->MPrint(static_cast<const char*>(outstr), cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null,cScrStr::Null);

        // Clamp the rate. The upper setting should never be needed, but check anyway
        if(rate < min_rate) rate = min_rate;
        if(rate > max_rate) rate = max_rate;

        // Now update the object
        cScrVec newprop;
        newprop.x = rate;
        newprop.y = low;
        newprop.z = high;
        prop = newprop;
        pPS -> Set(ObjId(), "CfgTweqRotate", propname, prop);
        }*/
}


/* -----------------------------------------------------------------------------
 *  Dark Engine message hooks
 */

long cScr_TWTweqSmooth::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
    if(pSimMsg -> fStarting) init();

    return cBaseScript::OnSim(pSimMsg, mpReply);
}


long cScr_TWTweqSmooth::OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply)
{
    if(!strcmp(pTimerMsg -> name, "TWRateUpdate") &&
       (pTimerMsg->data.type == kMT_String) &&
       !stricmp(pTimerMsg -> data.psz, "TWScripts")) {
        clear_timer();

        if(do_tweq_rotate) {
            for(int axis = 0; axis < RotAxisMax; ++axis) {
                if(axis_smooth[axis]) {
                    set_axis_rate(axis_names[axis], (RotAxis)axis);
                }
            }
        }

        start_timer();
    }

    return cBaseScript::OnTimer(pTimerMsg, mpReply);

}


/* =============================================================================
 *  TWTrapSetSpeed Impmementation
 */

/** A convenience structure used to pass speed and control
 *  data from the cScr_TWTrapSetSpeed::OnTurnOn() function
 *  to the link iterator callback.
 */
struct TWSetSpeedData
{
    float speed;     //!< The speed set by the user.
    bool  immediate; //!< Whether the speed change should be immediate.
};


int cScr_TWTrapSetSpeed::set_mterr_speed(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData)
{
    TWSetSpeedData *data = static_cast<TWSetSpeedData *>(pData);

    // Get the scriptparams link - dest should be a moving terrain object
    sLink current_link;
    pLQ -> Link(&current_link);
    object mterr_obj = current_link.dest; // For readability

    // Find out where the moving terrain is headed to
	SInterface<ILinkManager> pLM(g_pScriptManager);
    SInterface<IRelation> path_next_rel = pLM -> GetRelationNamed("TPathNext");

    // Try to get the link to the next waypoint
    long id = path_next_rel -> GetSingleLink(mterr_obj, 0);
    if(id != 0) {

        // dest in this link should be where the moving terrain is going
        sLink target_link;
        path_next_rel -> Get(id, &target_link);
        object terrpt_obj = target_link.dest;   // For readability

        if(terrpt_obj) {
            SService<IObjectSrv> pOS(g_pScriptManager);
            SService<IPhysSrv> pPS(g_pScriptManager);

            // Get the location of the terrpt
            cScrVec target_pos;
            cScrVec terrain_pos;
            pOS -> Position(target_pos, terrpt_obj);
            pOS -> Position(terrain_pos, mterr_obj);

            // Now work out what the velocity vector should be, based on the
            // direction to the target and the speed.
            cScrVec direction = target_pos - terrain_pos;
            if(direction.MagSquared() > 0.0001) {
                // The moving terrain is not on top of the terrpt
                direction.Normalize();
                direction *= data -> speed;
            } else {
                // On top of it, the game should pick this up and move the mterr to a
                // new path.
                direction = cScrVec::Zero;
            }

            // Set the speed. Note that UpdateMovingTerrainVelocity does something with
            // 'ClearTransLimits()' and 'AddTransLimit()' here - that seems to be something
            // to do with setting the waypoint trigger, so we should be okay to just update the
            // speed here as we're not changing the target waypoint.
            pPS -> ControlVelocity(mterr_obj, direction);
            if(data -> immediate) pPS -> SetVelocity(mterr_obj, direction);
        }
    }

    return 1;
}


void cScr_TWTrapSetSpeed::set_tpath_speed(object obj_id)
{
    SService<ILinkSrv> pLS(g_pScriptManager);
    SService<ILinkToolsSrv> pLTS(g_pScriptManager);

    // Convert to a multiparm here for ease
    cMultiParm setspeed = speed;

    // Fetch all TPath links from the specified object to any other
    linkset lsLinks;
    pLS -> GetAll(lsLinks, pLTS -> LinkKindNamed("TPath"), obj_id, 0);

    // Set the speed for each link to the set speed.
    for(; lsLinks.AnyLinksLeft(); lsLinks.NextLink()) {
        pLTS -> LinkSetData(lsLinks.Link(), "Speed", setspeed);
    }
}


void cScr_TWTrapSetSpeed::init()
{
    cAnsiStr my_name = get_object_namestr(ObjId());

    // Fetch the contents of the object's design note
    char *design_note = GetObjectParams(ObjId());

    if(!design_note)
        DebugPrintf("WARNING[TWTrapSetSpeed]: %s has no Editor -> Design Note. Falling back on defaults.", static_cast<const char *>(my_name));

    // Get the speed the user has set for this object (which could be a QVar, so this may be 0)
    speed = GetParamFloat(design_note, "TWTrapSetSpeed", 0.0f);

    // Is immediate mode enabled?
    immediate = GetParamBool(design_note, "TWTrapSetSpeedImmediate", false);

    // Is debugging mode enabled?
    debug = GetParamBool(design_note, "TWTrapSetSpeedDebug", false);

    // If the user has specified a QVar to use, copy the name
    char *qvar = GetParamString(design_note, "TWTrapSetSpeedQVar");
    if(qvar) {
        qvar_name = qvar;
        g_pMalloc -> Free(qvar);
    }

    // Sort out the target string too.
    // IMPORTANT NOTE: While it is tempting to build the full target object list at this point,
    // doing so may possibly miss dynamically created terrpts.
    char *target = GetParamString(design_note, "TWTrapSetSpeedDest", "[me]");
    if(target) {
        set_target = target;
        g_pMalloc -> Free(qvar);
    }
    if(!set_target) DebugPrintf("WARNING[TWTrapSetSpeed]: %s target set failed!", static_cast<const char *>(my_name));

    // If a design note was obtained, free it now
    if(design_note) g_pMalloc -> Free(design_note);

    // If debugging is enabled, print some Helpful Information
    if(debug) {
        DebugPrintf("DEBUG[TWTrapSetSpeed(OnSim)]: %s has initialised. Settings:\nSpeed: %.3f", static_cast<const char *>(my_name), speed);
        DebugPrintf("Immediate speed change: %s\n", immediate ? "enabled" : "disabled");
        if(qvar_name)  DebugPrintf("Speed will be read from QVar: %s\n", static_cast<const char *>(qvar_name));
        if(set_target) DebugPrintf("Targetting: %s\n", static_cast<const char *>(set_target));
    }
}


long cScr_TWTrapSetSpeed::OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply)
{
    SInterface<IObjectSystem> pOS(g_pScriptManager);
    cAnsiStr my_name = get_object_namestr(ObjId());

    if(debug)
        DebugPrintf("DEBUG[TWTrapSetSpeed]: %s has received a TurnOn.", static_cast<const char *>(my_name));

    // If the user has specified a QVar to use, read that
    if(qvar_name) {
    }

    if(debug)
        DebugPrintf("DEBUG[TWTrapSetSpeed]: %s using speed %.3f.", static_cast<const char *>(my_name), data.speed);

    // If a target has been parsed, fetch all the objects that match it
    if(set_target) {
        if(debug)
            DebugPrintf("DEBUG[TWTrapSetSpeed]: %s looking up targets matched by %s.", static_cast<const char *>(my_name), static_cast<const char *>(set_target));

        std::vector<object>* targets = get_target_objects(target, pMsg);

        if(!targets -> empty()) {
            // Process the target list, setting the speeds accordingly
            std::vector<object>::iterator it;
            cAnsiStr targ_name;
            for(it = targets -> begin() ; it < targets -> end(); it++) {
                set_tpath_speed(*it);

                if(debug) {
                    targ_name = get_object_namestr(*it);
                    DebugPrintf("DEBUG[TWTrapSetSpeed]: %s setting speed %.3f on %s.", static_cast<const char *>(my_name), speed, static_cast<const char *>(targ_name));
                }
            }
        } else {
            DebugPrintf("WARNING[TWTrapSetSpeed]: %s TWTrapSetSpeedDest '%s' did not match any objects.", static_cast<const char *>(my_name), target);
        }

        // And clean up
        delete targets;
    }

    // Copy the speed and immediate setting so it can be made available to the iterator
    TWSetSpeedData data;
    data.speed = speed;
    data.immediate = immediate;

    // And now update any moving terrain objects linked to this one via ScriptParams
    IterateLinks("ScriptParams", ObjId(), 0, set_mterr_speed, this, static_cast<void*>(&data));

	return cBaseTrap::OnTurnOn(pMsg, mpReply);
}


long cScr_TWTrapSetSpeed::OnSim(sSimMsg* pSimMsg, cMultiParm& mpReply)
{
    if(pSimMsg -> fStarting) init();

    return cBaseTrap::OnSim(pSimMsg, mpReply);
}
