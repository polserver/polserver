using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace POL.Log_Parser
{
    public class LogParser
    {
        public static ArrayList Results = new ArrayList();
        private static StringBuilder LogResults = new StringBuilder();
        private static string LogParsed = "";
        
        public static bool Parse()
        {
            if (Program.CommandLineFile == null)
            {
                // Before we try to parse, let's make sure CurrentList is in the Keywords
                // Dict. Someone could have deleted it in the editor!
                if (Program.Keys.KeywordDict.ContainsKey(Program.Keys.CurrentList) == false)
                {
                    MessageBox.Show("You must have deleted the Keyword List you was using in the editor! Please Refresh and try again.");
                    return false;
                }

                OpenFileDialog dialog = new OpenFileDialog();
                dialog.AddExtension = true;
                dialog.Filter = "Log files (*.log)|*.log|All files (*.*)|*.*";
                dialog.InitialDirectory = Program.opt.DefaultLogPath.ToString();
                if (dialog.ShowDialog() != DialogResult.OK)
                {
                    MessageBox.Show("Error Reading Log File or no Log File Selected!");
                    return false;
                }
                Program.CommandLineFile = dialog.FileName.ToString();
                dialog.Dispose();
            }

            StreamReader LogFile = File.OpenText(Program.CommandLineFile);
            LogParsed = Program.CommandLineFile;

            // LogLine is the text line read in
            // We use a StringBuilder here to build the output for the text window.
            string LogLine = "";

            // Let's reset these so they are clean! :)
            Results.Clear();

            while ((LogLine = LogFile.ReadLine()) != null)
            {
                foreach (string TheKeyword in Program.Keys.KeywordDict[Program.Keys.CurrentList])
                {
                    string NewLogLine = LogLine;
                    string NewKeyword = TheKeyword;
                    if (Program.opt.CaseParse)
                    {
                        NewLogLine = LogLine.ToLower();
                        NewKeyword = TheKeyword.ToLower();
                    }

                    if (NewLogLine.IndexOf(NewKeyword) != -1)
                    {
                        // We found a match!
                        AddMatch(TheKeyword, LogLine.ToString());
                    }
                }
            }
            LogFile.Close();
            return true;
        }

        private static void AddMatch(string Keyword, string LogLine)
        {
            ArrayList TempList = new ArrayList();
            TempList.Add(Keyword);
            TempList.Add(LogLine);
            Results.Add(TempList);
        }

        public static string buildParseTextBox()
        {
            if (LogResults.Length > 0)
            {
                LogResults.Remove(0, LogResults.Length);
            }
            foreach (ArrayList LogEntry in Results)
            {
                LogResults.Append("Keyword: " + LogEntry[0]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append("LogLine: " + LogEntry[1]);
                LogResults.Append(Environment.NewLine);
                LogResults.Append(Environment.NewLine);
            }
            return LogResults.ToString();
        }

        public static bool Save()
        {
            string FileExtension = null;
            if (Program.CommandLineResult == null)
            {
                SaveFileDialog dialog = new SaveFileDialog();
                dialog.AddExtension = true;
                dialog.FileName = "Parse_Results";
                dialog.Filter = "Log files (*.log)|*.log|XML Log files (*.xml)|*.xml";

                dialog.InitialDirectory = Program.opt.DefaultOutputPath.ToString();
                if (dialog.ShowDialog() != DialogResult.OK)
                {
                    MessageBox.Show("Error Saving Log File or No Filename Given!");
                    return false;
                }
                if (dialog.FilterIndex == 1)
                {
                    FileExtension = "log";
                }
                else if (dialog.FilterIndex == 2)
                {
                    FileExtension = "xml";
                }
                Program.CommandLineResult = dialog.FileName.ToString();
                dialog.Dispose();
            }
            if (FileExtension == null)
            {
                FileExtension = Program.CommandLineResult.Substring((Program.CommandLineResult.Length - 3));
            }

            if (FileExtension.Equals("log") || FileExtension.Equals("txt"))
            {
                StreamWriter LogFile = File.CreateText(Program.CommandLineResult.ToString());
                LogFile.Write(LogResults.ToString());
                LogFile.Close();
                return true;
            }
            else if (FileExtension.Equals("xml"))
            {
                // Save as XML instead!
                XmlDocument dom = new XmlDocument();
                XmlDeclaration decl = dom.CreateXmlDeclaration("1.0", "utf-8", null);
                dom.AppendChild(decl);
                XmlElement Root = dom.CreateElement("LogFileResults");
                XmlElement SheetInfo = dom.CreateElement("Sheet_Information");
                XmlElement Title = dom.CreateElement("Title");
                Title.InnerText = "POL Log Parser Results File";
                SheetInfo.AppendChild(Title);
                XmlElement LogFileElement = dom.CreateElement("LogFile");
                LogFileElement.InnerText = LogParser.LogParsed.ToString();
                SheetInfo.AppendChild(LogFileElement);
                XmlElement ParseDate = dom.CreateElement("Date");
                ParseDate.InnerText = DateTime.Now.ToString();
                SheetInfo.AppendChild(ParseDate);
                Root.AppendChild(SheetInfo);

                XmlElement ParseResultsElement = dom.CreateElement("Parse_Results");
                foreach (ArrayList ResultEntry in Results)
                {
                    XmlElement Instance = dom.CreateElement("Instance");
                    XmlElement KeywordLine = dom.CreateElement("Keyword");
                    KeywordLine.InnerText = ResultEntry[0].ToString();
                    Instance.AppendChild(KeywordLine);
                    XmlElement LogFileLine = dom.CreateElement("Log_Line");
                    LogFileLine.InnerText = ResultEntry[1].ToString();
                    Instance.AppendChild(LogFileLine);
                    ParseResultsElement.AppendChild(Instance);
                }
                Root.AppendChild(ParseResultsElement);
                dom.AppendChild(Root);
                dom.Save(Program.CommandLineResult.ToString());
                return true;
            }
            else
            {
                MessageBox.Show("Invalid Extension!");
                return false;
            }
        }

    }
}
