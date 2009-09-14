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
using System.Data;
using System.Text;
using System.Windows;
using System.Windows.Forms;
using System.Xml.Serialization;
using System.Xml;
using System.IO;

namespace POL.Log_Parser
{
    public partial class KeywordForm : Form
    {
        private string CurrentEditingList = "MasterList";
        public KeywordForm()
        {
            InitializeComponent();
            buildListBox();
            this.Text = "Keyword List - " + CurrentEditingList;
            addKeywordsToGrid();
        }

        private void addKeywordsToGrid()
        {
            if (Program.Keys.KeywordDict.ContainsKey(CurrentEditingList) == false)
            {
                CurrentEditingList = KeywordCollection.getFirstKeywordList();
            }
            foreach (string KeywordText in Program.Keys.KeywordDict[CurrentEditingList])
            {
                object[] KeywordObject = new object[] { KeywordText.ToString(), CurrentEditingList.ToString() };
                KeywordGrid.Rows.Add(KeywordObject);
            }
        }

        // Not only does this refresh out listbox that gives the names of our Keyword
        // Lists, but it also CLEANS UP after a save/load. We use it to do this, because
        // when we call it after save/load, where it saves a new list/perm removes an old
        // list or loads a different list it changes the GridView which means any unsaved
        private void buildListBox()
        {
            keywordListsBox.Enabled = false;
            string[] KeysList = new string[Program.Keys.KeywordDict.Keys.Count];
            Program.Keys.KeywordDict.Keys.CopyTo(KeysList, 0);
            // if it's not in the master list, get rid of it!
            // We use this longer method instead of just using Dictionary.ContainsKey()
            // because we link the KeyList to the DataSource also. If not for the
            // DataSource, we'd just check the ContainsKey.
            if (Array.IndexOf(KeysList, CurrentEditingList) == -1)
            {
                CurrentEditingList = KeysList[0].ToString();
            }
            keywordListsBox.DataSource = KeysList;
            keywordListsBox.SelectedItem = keywordListsBox.Items.IndexOf(CurrentEditingList);
            keywordListsBox.SelectedIndex = keywordListsBox.Items.IndexOf(CurrentEditingList);
            keywordListsBox.Enabled = true;
            this.Text = "Keyword List - " + CurrentEditingList;
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            // We clear here, in order to create a fresh list during save.
            string KeywordListName = KeywordGrid.Rows[0].Cells["List_Name"].Value.ToString();
            if (Program.Keys.KeywordDict.ContainsKey(KeywordListName))
            {
                Program.Keys.KeywordDict[KeywordListName].Clear();
            }
            else
            {
                Program.Keys.KeywordDict.Add(KeywordListName, new System.Collections.ArrayList());
            }
            // Now we must build the XML File itself. There are two things here that
            // MUST be done. Move all Grid Values to the Dictionary, then build off
            // the Dictionary. We use the ForEach wrapper because of the way it automatically
            // Iterates through the Key/Value. Much easier, less code, no manual
            // manipulation is required to step through the Dictionary. The ForEach will
            // handle all that for us in the background.

            int counter;
            for (counter = 0; counter < (KeywordGrid.Rows.Count); counter++)
            {
                if (KeywordGrid.Rows[counter].Cells["Keywords"].Value != null)
                {
                    if (KeywordGrid.Rows[counter].Cells["Keywords"].Value.ToString().Length != 0)
                    {
                        string KeywordText = KeywordGrid.Rows[counter].Cells["Keywords"].Value.ToString();
                        if (Program.Keys.KeywordDict[KeywordListName].Contains(KeywordText) == false)
                        {
                            Program.Keys.KeywordDict[KeywordListName].Add(KeywordText);
                        }
                    }
                }
            }
            
            // Now let's build the XML file, based off the Dictionary that is now up-to-date
            // from the Editor.
            string filepath = AppDomain.CurrentDomain.SetupInformation.ApplicationBase;

            string FileName = Path.Combine(filepath, "Keywords.xml");

            XmlDocument dom = new XmlDocument();
            XmlDeclaration decl = dom.CreateXmlDeclaration("1.0", "utf-8", null);
            dom.AppendChild(decl);
            XmlElement Root = dom.CreateElement("KeywordList");
            
            foreach (KeyValuePair<string, System.Collections.ArrayList> KVP in Program.Keys.KeywordDict)
            {
                foreach (string KeywordText in KVP.Value)
                {
                    if (KeywordText != null || KeywordText.Length != 0)
                    {
                        XmlElement elem = dom.CreateElement("Keyword");
                        elem.InnerText = KeywordText;
                        elem.SetAttribute("listname", KVP.Key.ToString());
                        Root.AppendChild(elem);

                    }
                }
            }

            dom.AppendChild(Root);
            dom.Save(FileName); 
            // Rebuild the list box in case we added a New List or deleted one :o
            buildListBox();
            // This will clear out empty cells left behind.
            KeywordGrid.Rows.Clear();
            addKeywordsToGrid();
        }

        private void resetButton_Click(object sender, EventArgs e)
        {
            KeywordGrid.Rows.Clear();
        }

        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void KeywordGrid_UserAddedRow(object sender, DataGridViewRowEventArgs e)
        {
            KeywordGrid.CurrentRow.Cells["List_Name"].Value = CurrentEditingList;
        }

        private void newListButton_Click(object sender, EventArgs e)
        {
            string CleanedText = newListNameTextBox.Text.Trim();
            if (CleanedText == "New List Name Here" || CleanedText == "" || CleanedText == null)
            {
                MessageBox.Show("You did not enter a New Keyword List Name!");
                return;
            }
            if (Program.Keys.KeywordDict.ContainsKey(CleanedText) == true || CurrentEditingList == CleanedText)
            {
                MessageBox.Show("That List name is already in use!");
                return;
            }
            CurrentEditingList = CleanedText;
            KeywordGrid.Rows.Clear();
            this.Text = "Keyword List - Creating a NEW List called " + CleanedText;
        }

        private void deleteListButton_Click(object sender, EventArgs e)
        {
            if (Program.Keys.KeywordDict.Count < 2)
            {
                MessageBox.Show("You cannot delete the last list!");
                return;
            }
            string message = "Are you sure you want to delete the List " + keywordListsBox.SelectedItem.ToString() + "?";
            string caption = "Delete List " + keywordListsBox.SelectedItem.ToString();
            MessageBoxButtons buttons = MessageBoxButtons.YesNo;
            DialogResult result;
            result = MessageBox.Show(message, caption, buttons);
            if (result == DialogResult.No)
            {
                MessageBox.Show("Canceled");
                return;
            }

            if (Program.Keys.KeywordDict.ContainsKey(keywordListsBox.SelectedItem.ToString()))
            {
                Program.Keys.KeywordDict.Remove(keywordListsBox.SelectedItem.ToString());
            }
            CurrentEditingList = KeywordCollection.getFirstKeywordList();
            KeywordGrid.Rows.Clear();
            addKeywordsToGrid();
            buildListBox();
            this.Text = "Keyword List - " + CurrentEditingList;
        }

        private void loadListButton_Click(object sender, EventArgs e)
        {
            if (keywordListsBox.SelectedItem.ToString() == CurrentEditingList.ToString())
            {
                return;
            }
            string message = "Are you sure you want to load the List " + keywordListsBox.SelectedItem.ToString() + "? Any unsaved data will be lost.";
            string caption = "Load List " + keywordListsBox.SelectedItem.ToString();
            MessageBoxButtons buttons = MessageBoxButtons.YesNo;
            DialogResult result;
            result = MessageBox.Show(message, caption, buttons);
            if (result == DialogResult.No)
            {
                MessageBox.Show("Canceled");
                return;
            }
            CurrentEditingList = keywordListsBox.SelectedItem.ToString();
            KeywordGrid.Rows.Clear();
            addKeywordsToGrid();
            buildListBox();
            this.Text = "Keyword List - " + CurrentEditingList;

        }

    }
}