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
    public class KeywordCollection
    {
        public Dictionary<string, ArrayList> KeywordDict = new Dictionary<string, ArrayList>();
        public string CurrentList = "MasterList";
        public KeywordCollection()
        {
            CurrentList = Program.opt.LastUsedList.ToString();
        }

        public static void Load()
        {
            // Clear it out first, get a fresh load.
            Program.Keys.KeywordDict.Clear();
            string path = AppDomain.CurrentDomain.SetupInformation.ApplicationBase;
            string FileName = Path.Combine(Path.GetDirectoryName(path), "Keywords.xml");
            if (!System.IO.File.Exists(FileName))
                return;

            XmlDocument KeywordList = new XmlDocument();
            KeywordList.Load(FileName);

            XmlElement root = KeywordList.DocumentElement;
            XmlNodeList elemList = root.GetElementsByTagName("Keyword");

            System.Collections.IEnumerator ienum = elemList.GetEnumerator();
            while (ienum.MoveNext())
            {
                XmlElement KeywordEntry = (XmlElement)ienum.Current;
                string KeywordText = KeywordEntry.InnerText.ToString();
                string KeywordListName = KeywordEntry.GetAttribute("listname");
                if (KeywordText != null && KeywordListName != null)
                {
                    // Next we add this to a Storage to read in the log parser.
                    try
                    {
                        Program.Keys.KeywordDict.Add(KeywordListName, new ArrayList());
                        Program.Keys.KeywordDict[KeywordListName].Add(KeywordText);
                    }
                    catch (ArgumentException)
                    {
                        if (Program.Keys.KeywordDict[KeywordListName].Contains(KeywordText) == false)
                        {
                            Program.Keys.KeywordDict[KeywordListName].Add(KeywordText);
                        }
                    }
                }
            }
        }

        // Does nothing more than makes sure the CurrentList is set back to 
        // Default. Which is the First Key in the Dictionary.
        public static void selectFirstKeywordList()
        {
            string[] KeysList = new string[Program.Keys.KeywordDict.Keys.Count];
            Program.Keys.KeywordDict.Keys.CopyTo(KeysList, 0);
            Program.Keys.CurrentList = KeysList[0].ToString();
        }

        // Does nothing more than returns the First Key in the Dictionary.
        public static string getFirstKeywordList()
        {
            string[] KeysList = new string[Program.Keys.KeywordDict.Keys.Count];
            Program.Keys.KeywordDict.Keys.CopyTo(KeysList, 0);
            return KeysList[0].ToString();
        }

    }
}
