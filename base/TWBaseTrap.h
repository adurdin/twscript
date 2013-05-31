/** @file
 * This file contains the interface for the base trap class. This class
 * adds trap-sepcific facilities, including the ability to remap the
 * TurnOn and TurnOff messages, limit the number of times the script
 * will perform the TurnOn and TurnOff actions, and other advanced trap
 * functions.
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

#ifndef TWBASETRAP_H
#define TWBASETRAP_H

#if !SCR_GENSCRIPTS

#include "TWBaseScript.h"
#include "SavedCounter.h"

class TWBaseTrap : public TWBaseScript
{
public:
    /* ------------------------------------------------------------------------
     *  Public interface exposed to the rest of the game
     */

    enum CountModes {
        CM_NOTHING = 0,
        CM_TURNON,
        CM_TURNOFF,
        CM_BOTH
    };

    /** Create a new TWBaseTrap object. This sets up a new TWBaseTrap object
     *  that is attached to a concrete object in the game world.
     *
     * @param name   The name of the script.
     * @param object The ID of the client object to add the script to.
     * @return A new TWBaseTrap object.
     */
    TWBaseTrap(const char* name, int object) : TWBaseScript(name, object),
                                               turnon_msg("TurnOn"), turnoff_msg("TurnOff"),
                                               count(name, object), count_mode(CM_BOTH),
                                               on_capacitor(name, object), off_capacitor(name, object)
        { /* fnord */ }

protected:
    /* ------------------------------------------------------------------------
     *  Message handling
     */

    /** Handle messages passed to the script. This is invoked whenever the
     *  script receives a message, and subclasses of this class will generally
     *  override or extend this function to provide script-specific behaviour.
     *
     * @param msg   A pointer to the message received by the object.
     * @param reply A reference to a multiparm variable in which a reply can
     *              be stored.
     * @return A status value indicating whether the caller should continue
     *         processing the message
     */
 	virtual MsgStatus on_message(sScrMsg* msg, cMultiParm& reply);


    /** Attempt to parse the count mode out of the specified design note.
     *  This handle situations where the user has set the CountOnly
     *  parameter to 0, 1, 2, 3, None, On, Off, or Both. Because it is
     *  nifty like that.
     *
     * @param design_note The design note to parse the count mode from.
     * @param default     The default CountMode to use if not set.
     * @return The selected (or default) count mode.
     */
    CountMode parse_countmode(char *design_note, CountMode default = CM_BOTH);


    /** Parse the value and falloff for a specified parameter from the
     *  design note. This tries to parse a value for the parameter,
     *  and a corresponding falloff if one has been specified.
     *
     * @param design_note The design note to parse the count mode from.
     * @param param       The name of the parameter to parse the value and
     *                    falloff for.
     * @param value       A pointer to an int to store the value in. The
     *                    int this points to will be set to 0 if no value
     *                    has been specified for the parameter. If you do
     *                    not need to parse a value, set this to NULL.
     * @param falloff     A pointer to an int to store the falloff value
     *                    in. If no value has been specified for the param,
     *                    the int pointed to by this will be set to 0. If
     *                    you do not need to parse a falloff, set this to NULL.
     */
    void parse_valuefalloff(char *design_note, const char *param, int *value = NULL, int *falloff = NULL);


private:
    /** Initialise the trap counters, message names, and other aspects of
     *  the trap class that couldn't be handled in the constructor. This
     *  should be called as part of processing BeginScript, before any
     *  attempt to use the class' features is made.
     *
     * @param time The current sim time.
     */
    void init(int time);


    // Message names
    cAnsiStr turnon_msg;        //!< The name of the message that should tigger the 'TurnOn' action
    cAnsiStr turnoff_msg;       //!< The name of the message that should tigger the 'TurnOff' action

    // Count handling
    SavedCounter count;         //!< Control how many times the script will work
    CountModes   count_mode;    //!< What counts as 'working'?

    // Capacitors
    SavedCounter on_capacitor;  //!< Control how frequently TurnOn actions work
    SavedCounter off_capacitor; //!< Control how frequently TurnOff actions work
};

#else // SCR_GENSCRIPTS
GEN_FACTORY("TWBaseTrap", "TWBaseScript", TWBaseTrap)
#endif // SCR_GENSCRIPTS

#endif // TWBASETRAP_H
