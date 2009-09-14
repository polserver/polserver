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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace POL.Log_Parser
{
    public partial class OptionsForm : Form
    {
        public OptionsForm()
        {
            InitializeComponent();
            this.defaultLogPathTextBox.Text = Program.opt.DefaultLogPath;
            this.defaultOutputPathBox.Text = Program.opt.DefaultOutputPath;
            this.defaultToolsPathTextBox.Text = Program.opt.DefaultPOLPath;
            this.caseSensitiveParseCheckbox.Checked = Program.opt.CaseParse;
        }

        private void defaultButton_Click(object sender, EventArgs e)
        {
            this.defaultLogPathTextBox.Text = "C:\\Logs";
        }

        private void browseButton_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.Description = "Select directory containing POL";
            dialog.ShowNewFolderButton = false;
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                this.defaultLogPathTextBox.Text = dialog.SelectedPath;
            }
        }

        private void defaultOutputPathButton_Click(object sender, EventArgs e)
        {
            this.defaultOutputPathBox.Text = "C:\\Logs";
        }

        private void browseOutputPathButton_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.Description = "Select directory for Result Output";
            dialog.ShowNewFolderButton = false;
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                this.defaultOutputPathBox.Text = dialog.SelectedPath;
            }
        }

        private void saveAllButton_Click(object sender, EventArgs e)
        {
            Program.opt.DefaultLogPath = this.defaultLogPathTextBox.Text;
            Program.opt.DefaultOutputPath = this.defaultOutputPathBox.Text;
            Program.opt.DefaultPOLPath = this.defaultToolsPathTextBox.Text;
            Program.opt.CaseParse = this.caseSensitiveParseCheckbox.Checked;
            Options.Save();
            this.Close();
        }

        private void resetButton_Click(object sender, EventArgs e)
        {
            this.defaultLogPathTextBox.Text = Program.opt.DefaultLogPath;
            this.defaultOutputPathBox.Text = Program.opt.DefaultOutputPath;
            this.defaultToolsPathTextBox.Text = Program.opt.DefaultPOLPath;
            this.caseSensitiveParseCheckbox.Checked = Program.opt.CaseParse;
        }

        private void defaultToolsPathButton_Click(object sender, EventArgs e)
        {
            this.defaultToolsPathTextBox.Text = "C:\\POL";
        }

        private void browseToolsPathButton_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.Description = "Select directory for Default POL Folder";
            dialog.ShowNewFolderButton = false;
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                this.defaultToolsPathTextBox.Text = dialog.SelectedPath;
            }
        }


    }
}