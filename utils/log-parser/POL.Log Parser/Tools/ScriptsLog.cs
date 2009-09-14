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
using System.Windows.Forms;

namespace POL.Log_Parser.Tools
{
    public partial class ScriptsLogForm : Form
    {
        public ScriptsLogForm()
        {
            InitializeComponent();
        }

        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void parseButton_Click(object sender, EventArgs e)
        {
            if (ScriptLogResults.Parse())
            {
                parseResultsTextBox.Text = "";
                parseResultsTextBox.Text = ScriptLogResults.buildParseTextBox();
                if (parseResultsTextBox.Text.Length > 8)
                {
                    this.saveButton.Enabled = true;
                }
            }
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            ScriptLogResults.Save();
        }
    }
}