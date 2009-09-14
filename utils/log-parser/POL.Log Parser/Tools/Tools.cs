/***************************************************************************
 *
 * $Author: MuadDib
 * 
 * "THE BEER-WARE LICENSE"
 * As long as you retain this notice you can do whatever you want with 
 * this stuff. If we meet some day, and you think this stuff is worth it,
 * you can buy me a beer in return.
 *
 ***************************************************************************/
using System;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Xml;

namespace POL.Log_Parser.Tools
{
    /***************************************************************************
    *
    *  Script.Log Tool Code
    * 
    ***************************************************************************/

    public class ScriptLogResults
    {
        private static ArrayList Results = new ArrayList();
        private static string ScriptLogParsed = "";
        private static StringBuilder LogResults = new StringBuilder();

        private static void AddScript(string Name, string LST)
        {
            ArrayList TempList = new ArrayList();
            TempList.Add(Name);
            TempList.Add(LST);
            Results.Add(TempList);
        }

       private static bool ContainsDup(string Needle)
        {
            foreach (ArrayList ScriptEntry in Results)
            {
                if (ScriptEntry.Contains(Needle))
                {
                    return true;
                }
            }

            return false;
        }

        public static bool Parse()
        {
            Results.Clear();
            
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.AddExtension = true;
            dialog.Filter = "Script Log files (script.log)|script.log";
            dialog.InitialDirectory = Program.opt.DefaultPOLPath.ToString();
            if (dialog.ShowDialog() != DialogResult.OK)
            {
                MessageBox.Show("Error Reading Log File or no Log File Selected!");
                return false;
            }

            StreamReader LogFile = File.OpenText(dialog.FileName.ToString());
            ScriptLogParsed = dialog.FileName.ToString();

            // LogLine is the text line read in
            // We use a StringBuilder here to build the output for the text window.
            string LogLine = "";
            StringBuilder LogResults = new StringBuilder();
            string ScriptName = "";
            bool FindLST = false;
            bool DupCheck = false;

            while ((LogLine = LogFile.ReadLine()) != null)
            {
                // Ok, we need to check LST.
                if (FindLST == true)
                {
                    // We need to keep going until we find the ">" at the first position
                    if (LogLine.IndexOf(">") == 0)
                    {
                        // Ok, this is the line. Let's see if a Dup was detected.
                        if (DupCheck == true)
                        {
                            if (ContainsDup(LogLine))
                            {
                                // This IS a dup entry! BOOOOO
                                ScriptName = "";
                                FindLST = false;
                                DupCheck = false;
                                continue;
                            }
                            else
                            {
                                // This is NOT a Dup LST entry! Let's add everything now
                                LogResults.Append("Script: " + ScriptName);
                                LogResults.Append(Environment.NewLine);
                                LogResults.Append(".LST Line: " + LogLine.ToString());
                                LogResults.Append(Environment.NewLine);
                                LogResults.Append(Environment.NewLine);
                                AddScript(ScriptName, LogLine);
                                ScriptName = "";
                            }
                        }
                        else
                        {
                            LogResults.Append(LogLine);
                            LogResults.Append(Environment.NewLine);
                            LogResults.Append(Environment.NewLine);
                            AddScript(ScriptName, LogLine);
                            ScriptName = "";
                        }
                        FindLST = false;
                        DupCheck = false;
                        continue;
                    }
                }
                else
                {
                    if (LogLine.IndexOf("Runaway script") != -1)
                    {
                        // We found a match!
                        LogLine = LogLine.Remove(0, (LogLine.IndexOf("]:") + 3));
                        LogLine = LogLine.Remove((LogLine.IndexOf(".ecl") + 4));
                        ScriptName = LogLine;

                        if (ContainsDup(ScriptName))
                        {
                            // HEY!!! We found a dup. Let's see if it's LST matches too!
                            FindLST = true;
                            DupCheck = true;
                            continue;
                        }
                        else
                        {
                            // Doesn't contain this script
                            LogResults.Append("Script: " + ScriptName);
                            LogResults.Append(Environment.NewLine);
                            LogResults.Append(".LST Line: ");
                            FindLST = true;
                            continue;
                        }
                    }
                }
            }

            dialog.Dispose();
            LogFile.Close();
            return true;
        }

        public static string buildParseTextBox()
        {
            if (LogResults.Length > 0)
            {
                LogResults.Remove(0, LogResults.Length);
            }
            foreach (ArrayList ScriptEntry in Results)
            {
                LogResults.Append("Script: " + ScriptEntry[0]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(".LST Line: " + ScriptEntry[1]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(Environment.NewLine);
            }
            return LogResults.ToString();
        }

        public static bool Save()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.AddExtension = true;
            dialog.FileName = "Script_Log_Results";
            dialog.Filter = "Log files (*.log)|*.log|XML Log files (*.xml)|*.xml";

            dialog.InitialDirectory = Program.opt.DefaultOutputPath.ToString();
            if (dialog.ShowDialog() != DialogResult.OK)
            {
                MessageBox.Show("Error Saving Log File or No Filename Given!");
                return false;
            }

            if (dialog.FilterIndex == 1)
            {
                StreamWriter LogFile = File.CreateText(dialog.FileName.ToString());
                LogFile.Write(LogResults.ToString());
                LogFile.Close();
            }
            else if (dialog.FilterIndex == 2)
            {
                // Save as XML instead!
                XmlDocument dom = new XmlDocument();
                XmlDeclaration decl = dom.CreateXmlDeclaration("1.0", "utf-8", null);
                dom.AppendChild(decl);
                XmlElement Root = dom.CreateElement("ScriptLogFileResults");
                XmlElement SheetInfo = dom.CreateElement("Sheet_Information");
                XmlElement Title = dom.CreateElement("Title");
                Title.InnerText = "POL Script.Log Parser Results File";
                SheetInfo.AppendChild(Title);
                XmlElement LogFileElement = dom.CreateElement("LogFile");
                LogFileElement.InnerText = ScriptLogParsed.ToString();
                SheetInfo.AppendChild(LogFileElement);
                XmlElement ParseDate = dom.CreateElement("Date");
                ParseDate.InnerText = DateTime.Now.ToString();
                SheetInfo.AppendChild(ParseDate);
                Root.AppendChild(SheetInfo);

                XmlElement ParseResultsElement = dom.CreateElement("Parse_Results");
                foreach (ArrayList ScriptInstance in Results)
                {
                    XmlElement Instance = dom.CreateElement("Instance");
                    XmlElement ScriptName = dom.CreateElement("Script");
                    ScriptName.InnerText = ScriptInstance[0].ToString();
                    Instance.AppendChild(ScriptName);
                    XmlElement LSTLine = dom.CreateElement("LST_Line");
                    LSTLine.InnerText = ScriptInstance[1].ToString();
                    Instance.AppendChild(LSTLine);
                    ParseResultsElement.AppendChild(Instance);
                }
                Root.AppendChild(ParseResultsElement);
                dom.AppendChild(Root);
                dom.Save(dialog.FileName.ToString());
            }

            dialog.Dispose();
            MessageBox.Show("Log Successfully Saved!");
            return true;
        }
     }

     /***************************************************************************
     *
     *  Debug.Log Tool Code
     * 
     ***************************************************************************/

    public class DebugLogResults
    {
        private static ArrayList ScriptResults = new ArrayList();
        private static ArrayList ConfigResults = new ArrayList();
        private static string DebugLogParsed = "";
        private static StringBuilder LogResults = new StringBuilder();

        private static void AddScript(string Name, string LST, string Function, string Param)
        {
            ArrayList TempList = new ArrayList();
            TempList.Add(Name);
            TempList.Add(LST);
            TempList.Add(Function);
            TempList.Add(Param);
            ScriptResults.Add(TempList);
        }

         private static void AddConfig(string Name)
        {
            ConfigResults.Add(Name);
        }

        // Check standard multi-dimensional arraylists
        private static bool ContainsDup(string Needle)
        {
            foreach (ArrayList ScriptEntry in ScriptResults)
            {
                if (ScriptEntry.Contains(Needle))
                {
                    return true;
                }
            }

            return false;
        }

        // Check the multi-dimensional arrays looking for both needle1 and 2
        // to confirm 2 entries are both a match! This can be used to check
        // ScriptName AND FunctioName or ScriptName AND LSTNumber. No Need
        // to check all three together since LSTNumber is unique per line
        // of code in a single script. Param Must be checked vs 2 others
        private static bool ContainsDup(string Needle1, string Needle2)
        {
            foreach (ArrayList ScriptEntry in ScriptResults)
            {
                if (ScriptEntry.Contains(Needle1) && ScriptEntry.Contains(Needle2))
                {
                    return true;
                }
            }

            return false;
        }

        // Check the multi-dimensional arrays looking for both needle1, 2 and 3
        // to confirm 3 entries are all a match! This can be used to check
        // ScriptName AND FunctioName AND Param
        private static bool ContainsDup(string Needle1, string Needle2, string Needle3)
        {
            foreach (ArrayList ScriptEntry in ScriptResults)
            {
                if (ScriptEntry.Contains(Needle1) && ScriptEntry.Contains(Needle2) && ScriptEntry.Contains(Needle3))
                {
                    return true;
                }
            }

            return false;
        }

        // Used to see if the config file is already stored.
        private static bool ContainsDup(string Needle, bool Config)
        {
            return ConfigResults.Contains(Needle);
        }

        public static bool Parse()
        {
            ScriptResults.Clear();
            ConfigResults.Clear();

            OpenFileDialog dialog = new OpenFileDialog();
            dialog.AddExtension = true;
            dialog.Filter = "Debug Log files (debug.log)|debug.log";
            dialog.InitialDirectory = Program.opt.DefaultPOLPath.ToString();
            if (dialog.ShowDialog() != DialogResult.OK)
            {
                MessageBox.Show("Error Reading Log File or no Log File Selected!");
                return false;
            }

            StreamReader LogFile = File.OpenText(dialog.FileName.ToString());
            DebugLogParsed = dialog.FileName.ToString();

            // LogLine is the text line read in
            // We use a StringBuilder here to build the output for the text window.
            string LogLine = "";
            string ScriptName = "";
            string LSTNumber = "";
            string FunctionName = "";

            bool FindFunction = false;
            bool FindParam = false;
            bool DupCheck = false;

            while ((LogLine = LogFile.ReadLine()) != null)
            {
                if (LogLine.IndexOf("Config File ") != -1)
                {
                    LogLine = LogLine.Remove(0, 12);
                    LogLine = LogLine.Remove(LogLine.IndexOf(" does not exist."));

                    if (ContainsDup(LogLine, true) == false)
                    {
                        AddConfig(LogLine.ToString());
                    }
                    continue;
                }

                if (FindFunction == true)
                {
                    if (LogLine.IndexOf("Call to function ") != -1)
                    {
                        LogLine = LogLine.Substring((LogLine.IndexOf("Call to function ") + 17));
                        LogLine = LogLine.Remove((LogLine.Length - 1));
                        FunctionName = LogLine.ToString();

                        // Ok, now let's check for a dup function :/
                        if (DupCheck == true)
                        {
                            if (ContainsDup(LogLine))
                            {
                                // We found it to be a dup. Go on to Param to check dup
                                FindFunction = false;
                                FindParam = true;
                            }
                            else
                            {
                                // Not a dup.
                                FindFunction = false;
                                FindParam = true;
                                DupCheck = false;
                            }
                        }
                        else
                        {
                            // no dup to check right now, parse as normal
                            FindFunction = false;
                            FindParam = true;
                        }
                    }
                    continue;
                }
                else if (FindParam == true)
                {
                    LogLine = LogLine.Trim();
                    if (DupCheck == true)
                    {
                        if (ContainsDup(ScriptName, FunctionName, LogLine))
                        {
                            // It's a complete dup. Clean Up and go on to the next Entry.
                            FindParam = false;
                            DupCheck = false;
                            ScriptName = "";
                            LSTNumber = "";
                            FunctionName = "";
                            continue;
                        }
                        else
                        {
                            // It's not a dup! Let's build all this crap now and clean up
                            AddScript(ScriptName, LSTNumber, FunctionName, LogLine);
                            FindParam = false;
                            DupCheck = false;
                            ScriptName = "";
                            LSTNumber = "";
                            FunctionName = "";
                        }
                    }
                    else
                    {
                        // We are not checking for a dup. Let's work as usual
                        AddScript(ScriptName, LSTNumber, FunctionName, LogLine);
                        FindParam = false;
                        ScriptName = "";
                        LSTNumber = "";
                        FunctionName = "";
                    }
                    continue;
                }
                else
                {
                    if (LogLine.IndexOf("Script Error in") != -1)
                    {
                        // We found a match!
                        LogLine = LogLine.Remove(0, 17);
                        LogLine = LogLine.Replace(".ecl'", ".src");
                        LogLine = LogLine.Remove(LogLine.IndexOf(":"));
                        LSTNumber = LogLine.Substring((LogLine.IndexOf("PC=") + 3));
                        LogLine = LogLine.Remove(LogLine.IndexOf(" PC="));
                        ScriptName = LogLine;
                        if (ContainsDup(ScriptName))
                        {
                            // HEY!!! We found a dup. Let's see if it's LST matches too!
                            if (ContainsDup(ScriptName, LSTNumber))
                            {
                                // LST Matches this scriptname already. It's a dup.
                                FindFunction = true;
                                DupCheck = true;
                                continue;
                            }
                            else
                            {
                                // This ScriptName does not contain a reference with
                                // this LST Number
                                FindFunction = true;
                                continue;
                            }
                        }
                        else
                        {
                            // This ScriptName hasn't been recorded yet
                            FindFunction = true;
                            continue;
                        }
                    }
                }
            }

            dialog.Dispose();
            LogFile.Close();
            return true;
        }

        public static string buildParseTextBox()
        {
            if (LogResults.Length > 0)
            {
                LogResults.Remove(0, LogResults.Length);
            }
            foreach (ArrayList ScriptEntry in ScriptResults)
            {
                LogResults.Append("Script: " + ScriptEntry[0]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(".LST Line: " + ScriptEntry[1]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append("Function: " + ScriptEntry[2]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append("Error: " + ScriptEntry[3]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(Environment.NewLine);
            }

            foreach (string ConfigEntry in ConfigResults)
            {
                LogResults.Append("Missing Config: " + ConfigEntry);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(Environment.NewLine);
            }

            return LogResults.ToString();
        }

        public static bool Save()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.AddExtension = true;
            dialog.FileName = "Debug_Log_Results";
            dialog.Filter = "Log files (*.log)|*.log|XML Log files (*.xml)|*.xml";

            dialog.InitialDirectory = Program.opt.DefaultOutputPath.ToString();
            if (dialog.ShowDialog() != DialogResult.OK)
            {
                MessageBox.Show("Error Saving Log File or No Filename Given!");
                return false;
            }

            if (dialog.FilterIndex == 1)
            {
                StreamWriter LogFile = File.CreateText(dialog.FileName.ToString());
                LogFile.Write(LogResults.ToString());
                LogFile.Close();
            }
            else if (dialog.FilterIndex == 2)
            {
                // Save as XML instead!
                XmlDocument dom = new XmlDocument();
                XmlDeclaration decl = dom.CreateXmlDeclaration("1.0", "utf-8", null);
                dom.AppendChild(decl);
                XmlElement Root = dom.CreateElement("DebugLogFileResults");
                XmlElement SheetInfo = dom.CreateElement("Sheet_Information");
                XmlElement Title = dom.CreateElement("Title");
                Title.InnerText = "POL Debug.Log Parser Results File";
                SheetInfo.AppendChild(Title);
                XmlElement LogFileElement = dom.CreateElement("LogFile");
                LogFileElement.InnerText = DebugLogParsed.ToString();
                SheetInfo.AppendChild(LogFileElement);
                XmlElement ParseDate = dom.CreateElement("Date");
                ParseDate.InnerText = DateTime.Now.ToString();
                SheetInfo.AppendChild(ParseDate);
                Root.AppendChild(SheetInfo);

                XmlElement ParseResultsElement = dom.CreateElement("Parse_Results");
                XmlElement ScriptResultsElement = dom.CreateElement("Script_Results");
                foreach (System.Collections.ArrayList ScriptInstance in ScriptResults)
                {
                    XmlElement Instance = dom.CreateElement("Script_Instance");
                    XmlElement ScriptName = dom.CreateElement("Script");
                    ScriptName.InnerText = ScriptInstance[0].ToString();
                    Instance.AppendChild(ScriptName);
                    XmlElement LSTLine = dom.CreateElement("LST_Line");
                    LSTLine.InnerText = ScriptInstance[1].ToString();
                    Instance.AppendChild(LSTLine);
                    ParseResultsElement.AppendChild(Instance);
                    XmlElement FunctionName = dom.CreateElement("Function_Name");
                    FunctionName.InnerText = ScriptInstance[2].ToString();
                    Instance.AppendChild(FunctionName);
                    ParseResultsElement.AppendChild(Instance);
                    XmlElement Error_Return = dom.CreateElement("Error_Return");
                    Error_Return.InnerText = ScriptInstance[3].ToString();
                    Instance.AppendChild(Error_Return);
                    ScriptResultsElement.AppendChild(Instance);
                }
                ParseResultsElement.AppendChild(ScriptResultsElement);

                XmlElement ConfigResultsElement = dom.CreateElement("Config_Results");
                foreach (string ConfigEntry in ConfigResults)
                {
                    XmlElement Instance = dom.CreateElement("Missing_Config");
                    XmlElement ConfigName = dom.CreateElement("Config");
                    ConfigName.InnerText = ConfigEntry;
                    Instance.AppendChild(ConfigName);
                    ConfigResultsElement.AppendChild(Instance);
                }
                ParseResultsElement.AppendChild(ConfigResultsElement);

                Root.AppendChild(ParseResultsElement);
                dom.AppendChild(Root);
                dom.Save(dialog.FileName.ToString());
            }

            dialog.Dispose();
            MessageBox.Show("Log Successfully Saved!");
            return true;
        }
    }

}
