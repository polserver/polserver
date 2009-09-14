using System;
using System.Windows.Forms;

namespace POL.Log_Parser.Tools
{
    public partial class DebugLogForm : Form
    {
        public DebugLogForm()
        {
            InitializeComponent();
        }

        private void closeButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void parseButton_Click(object sender, EventArgs e)
        {
            if (DebugLogResults.Parse())
            {
                parseResultsTextBox.Text = "";
                parseResultsTextBox.Text = DebugLogResults.buildParseTextBox();
                if (parseResultsTextBox.Text.Length > 8)
                {
                    this.saveButton.Enabled = true;
                }
            }
        }

        private void saveButton_Click(object sender, EventArgs e)
        {
            DebugLogResults.Save();
        }

    }
}