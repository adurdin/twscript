/** @file
 * This file contains the interface for the base trigger class. This class
 * adds trigger-sepcific facilities, including the ability to remap the
 * TurnOn and TurnOff messages sent when triggered, limit the number of
 * times the script will perform the TurnOn and TurnOff actions, and other
 * advanced trigger functions.
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

#ifndef TWBASETRIGGER_H
#define TWBASETRIGGER_H

#if !SCR_GENSCRIPTS

#include <string>
#include "TWBaseScript.h"
#include "SavedCounter.h"

class TWBaseTrigger : public TWBaseScript
{
public:
    /* ------------------------------------------------------------------------
     *  Public interface exposed to the rest of the game
     */

    /** Create a new TWBaseTrigger object. This sets up a new TWBaseTrigger object
     *  that is attached to a concrete object in the game world.
     *
     * @param name   The name of the script.
     * @param object The ID of the client object to add the script to.
     * @return A new TWBaseTrigger object.
     */
    TWBaseTrigger(const char* name, int object) : TWBaseScript(name, object),
                                                  messages { "TurnOff", "TurnOn" }, isstim { false, false }, stimob { 0, 0 }, intensity { 0.0f, 0.0f },
                                                  dest_str("&ControlDevice"),
                                                  remove_links(false),
                                                  fail_chance(0),
                                                  fail_qvar(),
                                                  count(name, object), count_mode(CM_BOTH),
                                                  uni_dist(0, 100)
        { /* fnord */ }

protected:
    /* ------------------------------------------------------------------------
     *  Initialisation related
     */

    /** Initialise the trigger counters, message names, and other aspects of
     *  the trigger class that couldn't be handled in the constructor. This
     *  should be called as part of processing BeginScript, before any
     *  attempt to use the class' features is made.
     *
     * @param time The current sim time.
     */
    virtual void init(int time);


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


    /** Send the defined 'On' message to the target objects.
     *
     * @return true if the message was sent, false otherwise.
     */
    bool send_on_message(sScrMsg* msg)
        { return send_trigger_message(true, msg); }


    /** Send the defined 'Off' message to the target objects.
     *
     * @return true if the message was sent, false otherwise.
     */
    bool send_off_message(sScrMsg* msg)
        { return send_trigger_message(false, msg); }


private:
    /* ------------------------------------------------------------------------
     *  Message handling
     */

    /** Send the trigger message to the trigger destination object(s). This should
     *  be called when the trigger fires to send the appropriate message to the
     *  destination object(s).
     *
     * @param send_on If true, this will send the 'on' message to the destination,
     *                otherwise it will send the 'off' message.
     * @return true if the message was sent, false otherwise.
     */
    bool send_trigger_message(bool send_on, sScrMsg* msg);


    /* ------------------------------------------------------------------------
     *  Miscellaneous
     */

    /** Determine whether the specified message is actually a stimulus request, and
     *  if so attempt to set up the settings based on the message.
     *
     */
    bool check_stimulus_message(char *message, int *obj, float *intensity);


    /* ------------------------------------------------------------------------
     *  Variables
     */

    // Message names
    std::string messages[2];  //!< The name of the message that should be sent as a 'turnon' or 'turnoff'

    // Stimulus for on/off
    bool  isstim[2];         //!< Is the turnon message a stimulus rather than a message?
    int   stimob[2];         //!< The stimulus object to use on turnon.
    float intensity[2];      //!< The stimulus intensity to use.

    // Destination setting
    std::string dest_str;    //!< Where should messages be sent?

    bool remove_links;       //!< Remove links after sending messages?

    // Full of fail?
    int fail_chance;         //!< percentage chance of the trigger failing.
    std::string fail_qvar;   //!< name of the qvar the fail percentage is in.

    // Count handling
    SavedCounter count;      //!< Control how many times the script will work
    CountMode    count_mode; //!< What counts as 'working'?

    // Randomness
    std::uniform_int_distribution<int> uni_dist;   //!< a uniform distribution for fail checking.
};

#else // SCR_GENSCRIPTS
GEN_FACTORY("TWBaseTrigger", "TWBaseScript", TWBaseTrigger)
#endif // SCR_GENSCRIPTS

#endif // TWBASETRIGGER_H
