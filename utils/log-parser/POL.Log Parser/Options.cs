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
using System.Collections.Generic;
using System.Text;
using System.Xml.Serialization;
using System.Xml;
using System.IO;
using System.Drawing;
using System.Windows.Forms.Design;
using System.ComponentModel;
using System.Collections;
using System.Windows.Forms;

namespace POL.Log_Parser
{
    public class Options
    {
        public string DefaultLogPath = "C:\\Logs";
        public string DefaultOutputPath = "C:\\Logs";
        public string DefaultPOLPath = "C:\\Pol";
        public string LastUsedList = "MasterList";
        public bool CaseParse = false;

        public Options()
        {
            
        }

        public static void Save()
        {
            string filepath = AppDomain.CurrentDomain.SetupInformation.ApplicationBase;

            string FileName = Path.Combine(filepath, "Options.xml");

            XmlDocument dom = new XmlDocument();
            XmlDeclaration decl = dom.CreateXmlDeclaration("1.0", "utf-8", null);
            dom.AppendChild(decl);
            XmlElement sr = dom.CreateElement("Options");

            XmlComment comment = dom.CreateComment("LogPath is the default path to check for log files when you press the Parse Button");
            sr.AppendChild(comment);
            XmlElement elem = dom.CreateElement("LogPath");
            elem.InnerText = Program.opt.DefaultLogPath.ToString();
            sr.AppendChild(elem);

            comment = dom.CreateComment("OutputPath is the default path to use when you press the Save Button after parsing");
            sr.AppendChild(comment);
            elem = dom.CreateElement("OutputPath");
            elem.InnerText = Program.opt.DefaultOutputPath.ToString();
            sr.AppendChild(elem);

            comment = dom.CreateComment("POLPath is the default path to use for the POL Parsing Tools in the Tools Menu");
            sr.AppendChild(comment);
            elem = dom.CreateElement("POLPath");
            elem.InnerText = Program.opt.DefaultPOLPath.ToString();
            sr.AppendChild(elem);

            comment = dom.CreateComment("LastUsedList is the last Keyword List used when Options was last saved");
            sr.AppendChild(comment);
            elem = dom.CreateElement("LastUsedList");
            elem.InnerText = Program.Keys.CurrentList.ToString();
            sr.AppendChild(elem);

            comment = dom.CreateComment("Case decides if to use Case Sensitive searches based on Keywords in the main Parser");
            sr.AppendChild(comment);
            elem = dom.CreateElement("Case");
            elem.InnerText = Program.opt.CaseParse.ToString();
            sr.AppendChild(elem);

            dom.AppendChild(sr);
            dom.Save(FileName);
        }

        public static void Load()
        {
            string path = AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string FileName = Path.Combine(Path.GetDirectoryName(path), "Options.xml");
            if (!System.IO.File.Exists(FileName))
                return;

            XmlDocument dom = new XmlDocument();
            dom.Load(FileName);
            XmlElement xOptions = dom["Options"];
            
            XmlElement elem = (XmlElement)xOptions.SelectSingleNode("LogPath");
            if (elem != null)
            {
                Program.opt.DefaultLogPath = elem.InnerText.ToString();
            }
            else
            {
                Program.opt.DefaultLogPath = ("C:\\Logs");
            }

            elem = (XmlElement)xOptions.SelectSingleNode("OutputPath");
            if (elem != null)
            {
                Program.opt.DefaultOutputPath = elem.InnerText.ToString();
            }
            else
            {
                Program.opt.DefaultOutputPath = ("C:\\Logs");
            }

            elem = (XmlElement)xOptions.SelectSingleNode("POLPath");
            if (elem != null)
            {
                Program.opt.DefaultPOLPath = elem.InnerText.ToString();
            }
            else
            {
                Program.opt.DefaultPOLPath = ("C:\\POL");
            }

            elem = (XmlElement)xOptions.SelectSingleNode("LastUsedList");
            if (elem != null)
            {
                Program.opt.LastUsedList = elem.InnerText.ToString();
                Program.Keys.CurrentList = Program.opt.LastUsedList;
            }
            else
            {
                Program.opt.LastUsedList = ("MasterList");
                Program.Keys.CurrentList = Program.opt.LastUsedList;
            }

            elem = (XmlElement)xOptions.SelectSingleNode("Case");
            if (elem != null)
            {
                Program.opt.CaseParse = bool.Parse(elem.InnerText.ToString());
            }
            else
            {
                Program.opt.CaseParse = false;
            }

        }
    }
}
