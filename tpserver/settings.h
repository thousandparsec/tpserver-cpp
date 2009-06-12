#ifndef SETTINGS_H
#define SETTINGS_H
/*  Settings class
 *
 *  Copyright (C) 2004-2005, 2006  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <string>
#include <map>

class SettingsCallback;

/**
 * Settings manager singleton
 */
class Settings {
  public:
    /**
     * Singleton accessor
     */
    static Settings *getSettings();

    /**
     * Read command-line args
     */
    bool readArgs(int argc, char** argv);

    /**
     * Read configuration file
     *
     * Config file name taken from stored config_file configuration
     *
     * @returns true if succeeded, false otherwise
     */
    bool readConfFile();

    /**
     * Set value explicitly
     */
    void set(std::string item, std::string value);

    /**
     * Return requested value
     */
    std::string get(std::string item);

    /**
     * Set a new configuiration item callback
     */
    void setCallback(std::string item, SettingsCallback cb);

    /**
     * Remove callback assigned to given item
     */
    void removeCallback(std::string item);

  private:
    /// Settings map typedef
    typedef std::map<std::string, std::string> SettingsMap;
    /// Callbacks map typedef
    typedef std::map<std::string, SettingsCallback> CallbackMap;

  private:
    /// Blocked default constructor
    Settings();
    /// Blocked destructor
    ~Settings();
    /// Blocked copy constructor
    Settings(Settings & rhs);
    /// Blocked assignment operator
    Settings operator=(Settings & rhs);

    /// Print help
    void printHelp();
    /// Log config error
    void gripeOnLine(const std::string& line, const char* complaint);
    /// Read configuration file
    bool readConfFile(const std::string& fname);

    void setDefaultValues();

    /// Settings storage
    SettingsMap store;
    /// Callback storage
    CallbackMap callbacks;

    /// Singleton instance
    static Settings *instance;

};

#endif
