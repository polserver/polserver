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
using System.Collections.Generic;
using System.Collections;
using System.Windows.Forms;
using System.Xml;

namespace POL.Log_Parser
{
    public class Program
    {
        public static Options opt = new Options();
        public static KeywordCollection Keys = new KeywordCollection();
        public static string Version = "1.0b6";
        public static string CommandLineFile = null;
        public static string CommandLineResult = null;
        
        [STAThread]
        static void Main(string[] args)
        {
            Options.Load();
            KeywordCollection.Load();
            if (args.Length > 0)
            {
                if (args.Length < 3)
                {
                    MessageBox.Show("You must provide at least a Keyword List, Input File, and Output File.");
                    Application.Exit();
                    return;
                }
                if (Program.Keys.KeywordDict.ContainsKey(args[0].ToString()))
                {
                    Program.Keys.CurrentList = args[0];
                }
                else
                {
                    MessageBox.Show("Keyword List does not exist in Keywords.XML.");
                    Application.Exit();
                    return;
                }
                if (File.Exists(args[1].ToString()))
                {
                    Program.CommandLineFile = args[1].ToString();
                }
                else
                {
                    MessageBox.Show("Log File does not exist.");
                    Application.Exit();
                    return;
                }
                if (File.Exists(args[2].ToString()))
                {
                    MessageBox.Show("Results File already exists! Appending Date....");
                    string TDate = DateTime.Now.ToFileTime().ToString();
                    Program.CommandLineResult = args[2].Insert((args[2].Length-4), TDate).ToString();
                }
                else
                {
                    Program.CommandLineResult = args[2].ToString();
                }
                if (args.Length > 3)
                {
                    try
                    {
                        Program.opt.CaseParse = bool.Parse(args[3].ToString());
                    }
                    catch (FormatException)
                    {
                        MessageBox.Show("Case Sensitive flag passed was not True or False!");
                        Application.Exit();
                        return;
                    }
                }
                // now let's parse this bad boy!
                if (LogParser.Parse())
                {
                    // We build this here because of the save handling
                    // to populate the LogResults Stream.
                    LogParser.buildParseTextBox();
                    LogParser.Save();
                }
                Application.Exit();
                return;
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new POLLogParserForm());
            Options.Save();
        }
    }
}