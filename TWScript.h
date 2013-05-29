/** @file
 * This file contains the interfaces for the TWScript script classes.
 *
 * @author Chris Page &lt;chris@starforge.co.uk&gt;
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef TWSCRIPT_H
#define TWSCRIPT_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#include "BaseTrap.h"

#include "scriptvars.h"
#include "darkhook.h"

#include <lg/scrservices.h>
#include <lg/links.h>
#include <vector>

/** AnimC bit field flags. No idea why this isn't in lg/defs.h, but hey...
 */
enum eAnimCFlags {
    kNoLimit   = 0x01, // ignore the limits (so I dont have to set them)
    kSim       = 0x02, // update me continually - else just update when I was on screen
    kWrap      = 0x04, // Wrap from low to high, else bounce
    k1Bounce   = 0x08, // Bounce off the top, then down and stop
    kSimSmall  = 0x10, // update if within a small radius
    kSimLarge  = 0x20, // update if within a large radius
    kOffScreen = 0x40  // only run if I'm offscreen
};


/** @class TWScript
 *
 * A class containing useful functions that various TW* scripts can use.
 *
 */
class TWScript
{
protected:
    /** Obtain a string containing the specified object's name (or archetype name),
     *  and its ID number. This has been lifted pretty much verbatim from Telliamed's
     *  Spy script - it is used to generate the object name and ID when writing
     *  debug messages.
     *
     * @param obj_id The ID of the object to obtain the name and number of.
     * @return A string containing the object name
     */
    static cAnsiStr get_object_namestr(object obj_id);


    /** Fetch the value in the specified QVar if it exists, return the default if it
     *  does not.
     *
     * @param qvar    The name of the QVar to return the value of.
     * @param def_val The default value to return if the qvar does not exist.
     * @return The QVar value, or the default specified.
     */
    long get_qvar_value(const char *qvar, long def_val);


    /** A somewhat more powerful version of get_qvar_value() that allows the
     *  inclusion of simple calculations to be applied to the value set in the
     *  QVar by including *value or /value in the string. For example, if the
     *  qvar variable contains 'foo/100' this will take the value in foo and
     *  divide it by 100. If the quest variable does not exist, this returns
     *  the default value specified *without applying any calculations to it*.
     *  The value may optionally be another QVar by placing $ before its name,
     *  eg: foo/$bar will divide the value in foo by the value in bar.
     *
     * @param qvar    The name of the QVar to return the value of, possibly including simple maths.
     * @param def_val The default value to return if the qvar does not exist.
     * @return The QVar value, or the default specified.
     */
    float get_qvar_value(const char *qvar, float def_val);


    /** Parse a string containing either a float value, or a qvar name, and
     *  return the float value contained in the string or qvar. See the docs
     *  for get_param_float for more information.
     *
     * @param param       A string to parse.
     * @param def_val     The default value to use if the string does not contain
     *                    a parseable value, or it references a non-existent QVar
     *                    (note that qvar_str will contain the QVar name, even if
     *                    the QVar does not exist)
     * @param qvar_str    A reference to a cAnsiStr to store the quest var name, or
     *                    quest var and simple calculation string.
     * @return The value specified in the string, or the float version of a value
     *         read from the qvar named in the string.
     */
    float parse_float(const char *param, float def_val, cAnsiStr &qvar_str);


    /** Read a float parameter from a design note string. If the value specified
     *  for the parameter in the design note is a simple number, this behaves
     *  identically to GetParamFloat(). However, this allows the user to specify
     *  the name of a QVar to read the value from by placing $ before the QVar
     *  name, eg: `ExampleParam='$a_quest_var'`. If a qvar is specified in this
     *  way, the user may also include the simple calculations supported by
     *  get_qvar_value(). If a QVar is specified - with or without additional
     *  calculations - the parameter string, with the leading $ removed, is stored
     *  in the provided qvar_str for later use.
     *
     * @param design_note The design note string to parse the parameter from.
     * @param name        The name of the parameter to parse.
     * @param def_val     The default value to use if the parameter does not exist,
     *                    or it references a non-existent QVar (note that qvar_str
     *                    will contain the parameter string even if the QVar does
     *                    not exist)
     * @param qvar_str    A reference to a cAnsiStr to store the quest var name, or
     *                    quest var and simple calculation string.
     * @return The value specified in the parameter, or the float version of a value
     *         read from the qvar named in the parameter.
     */
    float get_param_float(const char *design_note, const char *name, float def_val, cAnsiStr& qvar_str);


    /** Read a float vector (triple of three floats) from a design note string. This
     *  behaves in the same way as get_param_float(), except that instead of a single
     *  float value or QVar string, this expects three comma-separated float or QVar
     *  strings, one for each component of a vector (x, y, and z, in that order).
     *  If components are missing, this will use the specified default values
     *  instead.
     *
     * @param design_note The design note string to parse the parameter from.
     * @param name        The name of the parameter to parse.
     * @param vect        A reference to a vector to store the values in.
     * @param defx        The default value to use if no value has been set for the x component.
     * @param defy        The default value to use if no value has been set for the y component.
     * @param defz        The default value to use if no value has been set for the z component.
     * @return true if the named parameter <b>is present in the design note</b>. false
     *         if it is not. Note that this returns true even if the user has simply
     *         provided the parameter with no actual values, and defaults have been
     *         used for all the vector components. This should not be treated as indicating
     *         whether any values were parsed, rather it should be used to determine
     *         whether the parameter has been found.
     */
    bool get_param_floatvec(const char *design_note, const char *name, cScrVec &vect, float defx = 0.0f, float defy = 0.0f, float defz = 0.0f);


    /** Establish the length of the name of the qvar in the specified string. This
     *  will determine the length of the qvar name by looking for the end of the
     *  name string, or the presence of a simple calculation, and then working back
     *  until it hits the end of the name
     *
     * @param namestr A string containing a QVar name, and potentially a simple calculation.
     * @return The length of the QVar name, or 0 if the length can not be established.
     */
    int get_qvar_namelen(const char *namestr);


    /** Given a destination string, generate a list of object ids the destination
     *  corresponds to. If dest is '[me]', the current object is returned, if dest
     *  is '[source]' the source object is returned, if the dest is an object
     *  id or name, the id of that object is returned. If dest starts with * then
     *  the remainder of the string is used as an archetype name and all direct
     *  concrete descendents of that archetype are returned. If dest starts with
     *  @ then all concrete descendants (direct and indirect) are returned.
     *
     * @param pMsg A pointer to a script message containing the to and from objects.
     * @param dest The destination string
     * @return A vector of object ids the destination matches.
     */
    std::vector<object>* get_target_objects(const char *dest, sScrMsg *pMsg = NULL);

private:
    /** Determine whether the specified dest string is a radius search, and if so
     *  pull out its components. This will take a string like 5.00<Chest and set
     *  the radius to 5.0 and set the archetype string pointer to the start of the
     *  archetype name.
     *
     * @param dest      The dest string to check
     * @param radius    A pointer to a float to store the radius value in.
     * @param lessthan  A pointer to a bool. If the radius search is a < search
     *                  this is set to true, otherwise it is set to false.
     * @param archetype A pointer to a char pointer to set to the start of the
     *                  archetype name.
     * @return true if the dest string is a radius search, false otherwise.
     */
    bool radius_search(const char *dest, float *radius, bool *lessthan, const char **archetype);


    /** Search for concrete objects that are descendants of the specified archetype,
     *  either direct only (if do_full is false), or directly and indirectly. This
     *  can also filter the results based on the distance the concrete objects are
     *  from the specified object.
     *
     * @param matches   A pointer to the vector to store object ids in.
     * @param archetype The name of the archetype to search for. *Must not* include
     *                  and filtering (* or @) directives.
     * @param do_full   If false, only concrete objects that are direct descendants of
     *                  the archetype are matched. If true, all concrete objects that
     *                  are descendants of the archetype, or any descendant of that
     *                  archetype, are matched.
     * @param do_radius If false, concrete objects are matched regardless of distance
     *                  from the `from_obj`. If true, objects must be either inside
     *                  the specified radius from the `from_obj`, or outside out depending
     *                  on the `lessthan` flag.
     * @param from_obj  When filtering objects based on their distance, this is the
     *                  object that distance is measured from.
     * @param radius    The radius of the sphere that matched objects must fall inside
     *                  or outside.
     * @param lessthan  If true, objects must fall within the sphere around from_obj,
     *                  if false they must be outside it.
     */
    void archetype_search(std::vector<object> *matches, const char *archetype, bool do_full, bool do_radius = false, object from_obj = 0, float radius = 0.0f, bool lessthan = false);


    /** Split a string at a comma. This located the first comma in the specified string,
     *  replaces the comma with a null character, and returns a pointer to the character
     *  immediately following the replaced comma.
     *
     * @param src A pointer to a string to split at the first comma.
     * @return A pointer to the character immediately after a comma split, or NULL if
     *         the src string does not contain any commas.
     */
    char *comma_split(char *src);
};


/** @class cScr_TWTweqSmooth
 *
 * TWTweqSmooth allows the oscillating rotation of objects or joints to be
 * 'smoothed' over time, removing the hard, obvious direction changes otherwise
 * encountered. This can be used to create a number of different effects, but
 * is especially useful when simulating pendulum-like movement of objects or
 * subobjects.
 *
 * - the minimum tweq rate for each object axis or joint is determined by
 *   optional parameters set in the Editor -> Design Note. If minimum rates
 *   are not specified, a global minimum is used. If no global minimum has
 *   been specified, a fall-back default is used instead.
 * - the maximum tweq rate for each axis or joint is taken from the rate set
 *   in the Tweq -> Rotate or Tweq -> Joints settings. Essentially, the rate
 *   you would normally set to control a tweq in Dromed is used as the maximum
 *   rate for a smoothed tweq.
 * - if an axis or joint has a rate set that is less than or equal to the
 *   minimum rate, its movement will not be smoothed (ie: the maximum rate must
 *   be greater than the minimum rate).
 * - if an axis or joint has the same value for its low and high, or the low
 *   is greater than the high, its movement will not be smoothed.
 * - if the AnimC for a Tweq -> Rotate, or for any joint, has 'NoLimit' or
 *   'Wrap' set, no smoothing can be done.
 *
 * If warnings are enabled (see TWTweqSmoothWarn below), warnings will be written
 * to the monolog when the script has to disable smoothing on an axis or joint.
 *
 * Configuration
 * -------------
 * Parameters are specified using the Editor -> Design Note, please see the
 * main documentation for more about this.  Parameters supported by TWTweqSmooth
 * are listed below. If a parameter is not specified, the default value shown is
 * used instead. Note that all the parameters are optional, and if you do not
 * specify a parameter, the script will attempt to use a 'sane' default.
 *
 * Paramater: TWTweqSmoothTimerRate
 *      Type: integer
 *   Default: 250
 * The delay in milliseconds between tweq rate updates. This setting involves a
 * trade-off between performance and appearance: reducing this value (making the
 * delay between updates shorter) will make the rate adjustment smoother, but
 * it will also place more load on the engine. The default value is simply provided
 * as a starting point, and you will need to tweak it to suit the situation in
 * which the script is being used. Note that very small values should only be used
 * with Extreme Care.
 *
 * Parameter: TWTweqSmoothMinRate
 *      Type: real
 *   Default: 0.1
 * This allows you to set the default minimum rate for all the other rate controls.
 * For example, if you do not specify a value for TWTweqSmoothRotateXMin, the
 * script will use the value you set for TWTweqSmoothMinRate, falling back on the
 * built-in default of 0.1 if neither TWTweqSmoothMinRate or TWTweqSmoothRotateXMin
 * are set.
 *
 * Parameter: TWTweqSmoothRotate
 *      Type: string (comma separated values)
 *   Default: all
 * Provides control over the smoothing of rotation on different axes for objects that
 * have Tweq -> Rotate set. If not specified, all the axes are selected for smoothing.
 * If you provide a string, it should either be TWTweqSmoothRotate='none' to completely
 * turn off smoothing of rotation, or a comma separated list of axes to smooth rotation
 * on, for example TWTweqSmoothRotate='X,Z' will select the X and Z axes for smoothing.
 *
 * Parameter: TWTweqSmoothRotateXMin, TWTweqSmoothRotateYMin, TWTweqSmoothRotateZMin
 *      Type: real
 *   Default: TWTweqSmoothMinRate
 * Lets you individually set the minimum rates for each rotation axis. If an axis is not
 * set, the default minimum rate is used instead. If a minimum rate is specified for
 * an axis that is not selected for rotation smoothing by TWTweqSmoothRotate it will be
 * ignored.
 *
 * Parameter: TWTweqSmoothJoints
 *      Type: string (comma separated values)
 *   Default: all
 * Allows for control over the smoothing of individual joint movement on objects that
 * have Tweq -> Joints set. If not set, all joints
 */
class cScr_TWTweqSmooth : public cBaseScript, public TWScript
{
public:

    enum RotAxis {
        RotXAxis,
        RotYAxis,
        RotZAxis,
        RotAxisMax
    };

    enum RateMode {
        MinRate,
        MaxRate,
        RateModeMax
    };

	cScr_TWTweqSmooth(const char* pszName, int iHostObjId)
		: cBaseScript(pszName, iHostObjId)
    { }

protected:
	virtual long OnSim(sSimMsg*, cMultiParm&);
	virtual long OnTimer(sScrTimerMsg* pTimerMsg, cMultiParm& mpReply);

private:
    /** Determine whether the AnimC flags enabled in the specified value would
     *  prevent the correct smoothing of the tweq. This will check whether either
     *  the NoLimit or Wrap flags are set, and if they are it will return false
     *  (ie: incompatible).
     *
     * @param animc The integer containing the AnimC flags to check.
     * @return true if the flags are compatible with tweq smoothing, false otherwise.
     */
    inline bool compatible_animc(const int animc) {
        return !( animc & kNoLimit || animc & kWrap);
    };

    /** Fetch the rate set for a given axis on an object with Tweq -> Rotate set.
     *
     * @param axis The axis to fetch the rate for.
     * @return The tweq rate.
     */
    float get_rotate_rate(RotAxis axis);

    /** Determine whether the low and high values set for the specified axis on
     *  and object with Tweq -> Rotate set are valid. This will check that
     *  low and high are both positive, and that high is greater than low.
     *
     * @param axis The axis to check for bounds validity.
     * @return true if the axis bounds are valid, false otherwise.
     */
    bool valid_rotate_bounds(RotAxis axis);


    float get_joint_rate(int joint);
    float get_rate_param(const char *design_note, const char *cfgname, float default_value, float minimum = 0.0f);

    void init();
    int  init_rotate_onoffctrl(char *axes);
    void init_rotate(char *design_note);
    int  init_joints_onoffctrl(char *joints);
    void init_joints(char *design_note);

    void start_timer();
    void clear_timer();

    void set_axis_rate(const char *propname, RotAxis axis);

    bool      warnings;   //!< Show warning messages in monolog?

    // Timer-related variables.
    tScrTimer timer;      //!< The currently active timer for this object, or NULL.
    int       timer_rate; //!< The update rate, in milliseconds.

    // Default rates (which themselves have defaults!)
    float     min_rate;   //!< Default min rate for axes/joints that do not specify one
    float     max_rate;   //!< Default max rate for axes/joints that do not specify one

    // TweqRotate settings
    bool      do_tweq_rotate;                        //!< Should the rotate tweq be smoothed?
    bool      axis_smooth[RotAxisMax];               //!< Which axes should be smoothed?
    float     rotate_rates[RotAxisMax][RateModeMax]; //!< Store the per-axis rate min/max values

    // TweqJoints settings
    /** Anonymous enum hack, used to avoid the need for another static const int
     *  just to define the joint_* array sizes.
     */
    enum {
        JointCount = 6  //!< How many joints should we support? Dark supports 6.
    };
    bool      do_tweq_joints;                       //!< Should the joint tweq be smoothed?
    bool      joint_smooth[JointCount];             //!< Should individual joints be smoothed?
    float     joint_rates[JointCount][RateModeMax]; //!< Store the per-joint rate min/max values.

    static const char *axis_names[RotAxisMax];
    static const char *joint_names[JointCount];
};


/** @class cScr_TWTrapSetSpeed
 *
 * TWTrapSetSpeed allows the game-time modification of TPath speed settings.
 * This script lets you control how fast a vator moves between TerrPts on the
 * fly - add it to an object, set the TWTrapSetSpeed and TWTrapSetSpeedDest params
 * documented below, and then send a TurnOn message to the object when you
 * want it to apply the speed to the destination.
 *
 * By default, the speed changes made by this script will not be picked up by
 * and moving terrain objects moving between TerrPts until they reach their next
 * waypoint. However, if you want the speed of any moving terrain object to be
 * updated by this script before it reaches the next TerrPt, link the object
 * this script is placed on to the moving terrain object with a ScriptParams link,
 * and set the data for the link to "SetSpeed". This link is needed to get the moving
 * terrain to start moving from a stop (speed = 0).
 *
 * Configuration
 * -------------
 * Parameters are specified using the Editor -> Design Note, please see the
 * main documentation for more about this.  Parameters supported by TWTrapSetSpeed
 * are listed below. If a parameter is not specified, the default value shown is
 * used instead. Note that all the parameters are optional, and if you do not
 * specify a parameter, the script will attempt to use a 'sane' default.
 *
 * Parameter: TWTrapSetSpeed
 *      Type: float
 *   Default: 0.0
 * The speed to set the target objects' TPath speed values to when triggered. All
 * TPath links on the target object are updated to reflect the speed given here.
 * The value provided for this parameter may be taken from a QVar by placing a $
 * before the QVar name, eg: `TWTrapSetSpeed='$speed_var'`. If you set a QVar
 * as the speed source in this way, each time the script receives a TurnOn, it
 * will read the value out of the QVar and then copy it to the destination object(s).
 * Using a simple QVar as in the previous example will restrict your speeds to
 * integer values; if you need fractional speeds, you can include a simple
 * calculation after the QVar name to scale it, for example,
 * `TWTrapSetSpeed='$speed_var / 10'` will divide the value in speed_var by 10,
 * so if `speed_var` contains 55, the speed set by the script will be 5.5. You
 * can even specify a QVar as the second operand if needed, again by prefixing
 * the name with '$', eg: `TWTrapSetSpeed='$speed_var / $speed_div'`.
 *
 * Parameter: TWTrapSetSpeedWatchQVar
 *      Type: boolean
 *   Default: false
 * If TWTrapSetSpeed is set to read the speed from a QVar, you can make the
 * script trigger whenever the QVar is changed by setting this to true. Note
 * that this will only watch changes to the first QVar specified in TWTrapSetSpeed:
 * if you set `TWTrapSetSpeed='$speed_var / $speed_div'` then changes to speed_var
 * will be picked up, but any changes to speed_div will not trigger this script.
 *
 * Parameter: TWTrapSetSpeedDest
 *      Type: string
 *   Default: [me]
 * Specify the target object(s) to update when triggered. This can either be
 * an object name, [me] to update the object the script is on, [source] to update
 * the object that triggered the change (if you need that, for some odd reason),
 * or you may specify an archetype name preceeded by * or @ to update all objects
 * that inherit from the specified archetype. If you use *Archetype then only
 * concrete objects that directly inherit from that archetype are updated, if you
 * use @Archetype then all concrete objects that inherit from the archetype
 * directly or indirectly are updated.
 *
 * Parameter: TWTrapSetSpeedDebug
 *      Type: boolean
 *   Default: false
 * If this is set to true, debugging messages will be written to the monolog to help
 * trace problems with the script. Note that if you set this parameter to true, and
 * see no new output in the monolog, double-check that you have twscript loaded!
 *
 * Parameter: TWTrapSetSpeedImmediate
 *      Type: boolean
 *   Default: false
 * If this is set to true, the speed of any linked moving terrain objects is immediately
 * set to the speed value applied to the TerrPts. If it is false, the moving terrain
 * object will smoothly change its speed to the new speed (essentially, setting this
 * to true breaks the appearance of momentum and inertia on the moving object. It is
 * very rare that you will want to set this to true.)
 */
class cScr_TWTrapSetSpeed : public cBaseTrap, public TWScript
{
public:
    cScr_TWTrapSetSpeed(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
    { }

protected:
    /** TurnOn message handler, called whenever the script receives a TurnOn message.
     */
	virtual long OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply);

    /**
     */
    virtual long OnBeginScript(sScrMsg* pMsg, cMultiParm& mpReply);

    /**
     */
    virtual long OnEndScript(sScrMsg* pMsg, cMultiParm& mpReply);

    /** OnQuestChange handler, called whenever the questvar the script has subscribed
     *  to is updated. Note that this will only trigger speed updates if the qvar has
     *  actually changed, otherwise it will ignore the message.
     */
    virtual long OnQuestChange(sQuestMsg* pQuestMsg, cMultiParm& mpReply);

private:
    /** Initialise the TWTrapSetSpeed instance. This parses the various
     *  parameters from the design note, and sets up the script so that
     *  it can be used correctly.
     */
    void init();

    /** Update the speed set on any selected destination object(s) and linked
     *  moving terrain object(s). This is the function that does most of the
     *  work of actually updating TerrPts and so on to reflect the currently
     *  set speed. It will update the speed setting if the TWTrapSetSpeed
     *  design note parameter contains a QVar.
     *
     * @param pMsg A pointer to the message that triggered the update.
     */
    void update_speed(sScrMsg* pMsg);

    /** Update the speed set on an individual TerrPt's TPath links.
     *
     * @param obj_id The TerrPt object to update the TPath links on.
     */
    void set_tpath_speed(object obj_id);

    /** Link iterator callback used to set the speed of moving terrain objects.
     *  This allows the speed of moving terrain objects to be set on the fly,
     *  either with immediate effect or allowing the physics system to change
     *  the speed smoothly.
     *
     * @param pLQ   A pointer to the link query for the current call.
     * @param pData A pointer to a structure containing the speed and other settings.
     * @return Always returns 1.
     */
    static int set_mterr_speed(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData);

    float    speed;      //!< User-defined speed to set on targets and linked vators.
    bool     debug;      //!< If true, additional debugging output is shown.
    bool     immediate;  //!< If true, vator speed changes are instant.
    cAnsiStr qvar_name;  //!< The name of the QVar to read speed from, may include basic maths.
    cAnsiStr qvar_sub;   //!< The name of the QVar to subscribe to.
    cAnsiStr set_target; //!< The target string set by the user.
};



/** @class cScr_TWTrapPhysStateControl
 *
 * TWTrapPhysStateControl provides direct control over the location, orientation,
 * velocity, and rotational velocity of objects in Thief 2. Note that this script
 * provides a means to set the physics state values, but the game may ignore these
 * values in some situations, and any changes you make will be subsequently subject
 * to the normal physics simulation performed by the game (so, for example, changing
 * an object's position may result in it either staying in the new location, or
 * falling to (or through!) the ground, depending on how the object has been set up).
 *
 * Expect to have to experiment with this script!
 *
 * Add this script to a marker, link the marker to the object(s) whose physics
 * state you want to control using ControlDevice links. Whenever the marker is sent
 * a TurnOn message, the script will update the physics state of the objects linked
 * to the marker.
 *
 * *NOTE*: during testing, I was unable to reliably get the heading component of
 * rotational velocity to do anything. The value is going in fine, just none of my
 * tests seemed to be able to get a response to it - whether this is an error
 * in my code, tests, or the engine ignoring the value set I don't currently know.
 *
 * Configuration
 * -------------
 * Parameters are specified using the Editor -> Design Note, please see the
 * main documentation for more about this.  Parameters supported by
 * TWTrapPhysStateControl are listed below. If a parameter is not specified,
 * the default value shown is used instead. Note that all the parameters are
 * optional, and if you do not specify a parameter, the script will attempt to use
 * a 'sane' default.
 *
 * Parameter: TWTrapPhysStateCtrlLocation
 *      Type: float vector
 *   Default: none (location is not changed)
 * Set the location of the controlled object(s) the position specified. If this
 * parameter is not specified, the location of the object(s) is not modified. If you
 * specify this parameter, but give it no value (ie: `TWTrapPhysStateCtrlLocation=;`)
 * then the default location of `0, 0, 0` is used.
 *
 * Parameter: TWTrapPhysStateCtrlFacing
 *      Type: float vector
 *   Default: none (orientation is not changed)
 * Set the orientation of the controlled object(s) the values specified. If this
 * parameter is not specified, the orientation of the object(s) is not modified. If you
 * specify this parameter, but give it no value (ie: `TWTrapPhysStateCtrlFacing=;`)
 * then the default orientation of `0, 0, 0` is used. *IMPORTANT NOTE*: the values
 * specified for this parameter match the order found in Physics -> Model -> State,
 * so the first value is bank (B), the second is pitch (P), and the third is
 * heading (H). This is the opposite of the order most people would expect; if you
 * find yourself having problems orienting objects, check that you haven't mixed up
 * the bank and heading!
 *
 * Parameter: TWTrapPhysStateCtrlVelocity
 *      Type: float vector
 *   Default: none (velocity is not changed)
 * Set the velocity of the controlled object(s) the values specified. If this
 * parameter is not specified, the velocity of the object(s) is not modified. If you
 * specify this parameter, but give it no value (ie: `TWTrapPhysStateCtrlVelocity=;`)
 * then the default velocity of `0, 0, 0` is used.
 *
 * Parameter: TWTrapPhysStateCtrlRotVel
 *      Type: float vector
 *   Default: none (rotational velocity is not changed)
 * Set the rotational velocity of the controlled object(s) the values specified. If
 * this parameter is not specified, the rotational velocity of the object(s) is not
 * modified. If you specify this parameter, but give it no value
 * (ie: `TWTrapPhysStateCtrlRotVel=;`) then the default of `0, 0, 0` is used. Note
 * that, as with TWTrapPhysStateCtrlFacing, the first value of the vector is the
 * bank, the second is the pitch, and the third is the heading.
 *
 * Parameter: TWTrapPhysStateCtrlDebug
 *      Type: boolean
 *   Default: false
 * If this is set to true, debugging messages will be written to the monolog to help
 * trace problems with the script. Note that if you set this parameter to true, and
 * see no new output in the monolog, double-check that you have twscript loaded!
 */
class cScr_TWTrapPhysStateControl : public cBaseTrap, public TWScript
{
public:
    cScr_TWTrapPhysStateControl(const char* pszName, int iHostObjId)
		: cBaseTrap(pszName, iHostObjId)
    { }

protected:
    /** TurnOn message handler, called whenever the script receives a TurnOn message.
     */
	virtual long OnTurnOn(sScrMsg* pMsg, cMultiParm& mpReply);

private:
    /** Update the TWTrapPhysStateControl instance. This parses the various
     *  parameters from the design note, and updates the linked object(s).
     */
    void update();


    /** Link iterator callback used to set the physics state of objects. This
     *  is used to set the location, facing, velocity, and rotational velocity
     *  of objects linked to the script object via ControlDevice links.
     *
     * @param pLQ   A pointer to the link query for the current call.
     * @param pData A pointer to a structure containing the physics state settings.
     * @return Always returns 1.
     */
    static int set_state(ILinkSrv*, ILinkQuery* pLQ, IScript*, void* pData);
};

#else // SCR_GENSCRIPTS

GEN_FACTORY("TWTweqSmooth"          , "BaseScript", cScr_TWTweqSmooth)
GEN_FACTORY("TWTrapSetSpeed"        , "BaseTrap"  , cScr_TWTrapSetSpeed)
GEN_FACTORY("TWTrapPhysStateControl", "BaseTrap"  , cScr_TWTrapPhysStateControl)

#endif // SCR_GENSCRIPTS

#endif // TWSCRIPT_H
