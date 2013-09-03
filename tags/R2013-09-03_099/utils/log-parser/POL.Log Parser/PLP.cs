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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;

namespace POL.Log_Parser
{
    public partial class POLLogParserForm : Form
    {
        public POLLogParserForm()
        {
            InitializeComponent();
            buildListBoxForParse();
        }

        private void exit_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void closeProgram_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private AboutBox m_About = new AboutBox();
        private void about_Click(object sender, EventArgs e)
        {
            if (m_About.IsDisposed)
                m_About = new AboutBox();
            else
                m_About.Focus();
            m_About.TopMost = true;
            m_About.Show();
        }

        private KeywordForm m_KeywordForm = new KeywordForm();
        private void keyword_Click(object sender, EventArgs e)
        {
            if (m_KeywordForm.IsDisposed)
                m_KeywordForm = new KeywordForm();
            else
                m_KeywordForm.Focus();
            m_KeywordForm.TopMost = true;
            m_KeywordForm.Show();
        }

        private OptionsForm m_Options = new OptionsForm();
        private void optionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_Options.IsDisposed)
                m_Options = new OptionsForm();
            else
                m_Options.Focus();
            m_Options.TopMost = true;
            m_Options.Show();
        }

        private Tools.ScriptsLogForm m_ScriptsLog = new Tools.ScriptsLogForm();
        private void scriptslogToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_ScriptsLog.IsDisposed)
                m_ScriptsLog = new Tools.ScriptsLogForm();
            else
                m_ScriptsLog.Focus();
            m_ScriptsLog.TopMost = true;
            m_ScriptsLog.Show();
        }

        private Tools.DebugLogForm m_DebugLog = new Tools.DebugLogForm();
        private void debuglogToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_DebugLog.IsDisposed)
                m_DebugLog = new Tools.DebugLogForm();
            else
                m_DebugLog.Focus();
            m_DebugLog.TopMost = true;
            m_DebugLog.Show();
        }

        private void parseButton_Click(object sender, EventArgs e)
        {
            if (LogParser.Parse())
            {
                parseResultsBox.Text = "";
                parseResultsBox.Text = LogParser.buildParseTextBox();
                // 8 just sounds nice. Means something screwed up and at the least
                // there was no Keyword: added to the string.
                if (parseResultsBox.Text.Length > 8)
                {
                    this.saveButton.Enabled = true;
                    this.saveAsToolStripMenuItem.Enabled = true;
                    Program.CommandLineFile = null;
                }
            }

        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            if (LogParser.Save())
            {
                Program.CommandLineResult = null;
                MessageBox.Show("Log Successfully Saved!");
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (LogParser.Save())
            {
                Program.CommandLineResult = null;
                MessageBox.Show("Log Successfully Saved!");
            }
        }

        private void buildListBoxForParse()
        {
            listBoxForParse.Enabled = false;
            string[] KeysList = new string[Program.Keys.KeywordDict.Keys.Count];
            Program.Keys.KeywordDict.Keys.CopyTo(KeysList, 0);
            // We have this here, in case you deleted our current list in the editor!
            if (Array.IndexOf(KeysList, Program.Keys.CurrentList) == -1)
            {
                Program.Keys.CurrentList = KeysList[0].ToString();
            }
            listBoxForParse.DataSource = KeysList;
            listBoxForParse.SelectedItem = listBoxForParse.Items.IndexOf(Program.Keys.CurrentList.ToString());
            listBoxForParse.SelectedIndex = listBoxForParse.Items.IndexOf(Program.Keys.CurrentList.ToString());
            listBoxForParse.Enabled = true;
        }

        private void listBoxForParse_SelectedIndexChanged(object sender, EventArgs e)
        {
            Program.Keys.CurrentList = listBoxForParse.SelectedItem.ToString();
        }

        private void refreshListBox_Click(object sender, EventArgs e)
        {
            buildListBoxForParse();
        }

    }
}