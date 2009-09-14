namespace POL.Log_Parser
{
    partial class POLLogParserForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(POLLogParserForm));
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pOLLogFilesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.debuglogToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scriptslogToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.settingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.optionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.keywordListToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.panel1 = new System.Windows.Forms.Panel();
            this.refreshListBox = new System.Windows.Forms.Button();
            this.listBoxForParse = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.closeProgram = new System.Windows.Forms.Button();
            this.parseButton = new System.Windows.Forms.Button();
            this.parseResultsBox = new System.Windows.Forms.TextBox();
            this.saveButton = new System.Windows.Forms.Button();
            this.logParserTooltip = new System.Windows.Forms.ToolTip(this.components);
            this.menuStrip1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.SystemColors.Control;
            this.menuStrip1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1,
            this.toolsToolStripMenuItem,
            this.settingsToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.ShowItemToolTips = true;
            this.menuStrip1.Size = new System.Drawing.Size(531, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveAsToolStripMenuItem,
            this.exitToolStripMenuItem});
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(35, 20);
            this.toolStripMenuItem1.Text = "File";
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Enabled = false;
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.saveAsToolStripMenuItem.Text = "Save As";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(124, 22);
            this.exitToolStripMenuItem.Text = "&Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exit_Click);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.pOLLogFilesToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.toolsToolStripMenuItem.Text = "Tools";
            // 
            // pOLLogFilesToolStripMenuItem
            // 
            this.pOLLogFilesToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.debuglogToolStripMenuItem,
            this.scriptslogToolStripMenuItem});
            this.pOLLogFilesToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("pOLLogFilesToolStripMenuItem.Image")));
            this.pOLLogFilesToolStripMenuItem.Name = "pOLLogFilesToolStripMenuItem";
            this.pOLLogFilesToolStripMenuItem.Size = new System.Drawing.Size(148, 22);
            this.pOLLogFilesToolStripMenuItem.Text = "POL Log Files";
            // 
            // debuglogToolStripMenuItem
            // 
            this.debuglogToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("debuglogToolStripMenuItem.Image")));
            this.debuglogToolStripMenuItem.Name = "debuglogToolStripMenuItem";
            this.debuglogToolStripMenuItem.Size = new System.Drawing.Size(134, 22);
            this.debuglogToolStripMenuItem.Text = "debug.log";
            this.debuglogToolStripMenuItem.Click += new System.EventHandler(this.debuglogToolStripMenuItem_Click);
            // 
            // scriptslogToolStripMenuItem
            // 
            this.scriptslogToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("scriptslogToolStripMenuItem.Image")));
            this.scriptslogToolStripMenuItem.Name = "scriptslogToolStripMenuItem";
            this.scriptslogToolStripMenuItem.Size = new System.Drawing.Size(134, 22);
            this.scriptslogToolStripMenuItem.Text = "scripts.log";
            this.scriptslogToolStripMenuItem.Click += new System.EventHandler(this.scriptslogToolStripMenuItem_Click);
            // 
            // settingsToolStripMenuItem
            // 
            this.settingsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.optionsToolStripMenuItem,
            this.keywordListToolStripMenuItem});
            this.settingsToolStripMenuItem.Name = "settingsToolStripMenuItem";
            this.settingsToolStripMenuItem.Size = new System.Drawing.Size(58, 20);
            this.settingsToolStripMenuItem.Text = "Settings";
            // 
            // optionsToolStripMenuItem
            // 
            this.optionsToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("optionsToolStripMenuItem.Image")));
            this.optionsToolStripMenuItem.Name = "optionsToolStripMenuItem";
            this.optionsToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.optionsToolStripMenuItem.Text = "Options";
            this.optionsToolStripMenuItem.Click += new System.EventHandler(this.optionsToolStripMenuItem_Click);
            // 
            // keywordListToolStripMenuItem
            // 
            this.keywordListToolStripMenuItem.Image = ((System.Drawing.Image)(resources.GetObject("keywordListToolStripMenuItem.Image")));
            this.keywordListToolStripMenuItem.Name = "keywordListToolStripMenuItem";
            this.keywordListToolStripMenuItem.Size = new System.Drawing.Size(146, 22);
            this.keywordListToolStripMenuItem.Text = "&Keyword List";
            this.keywordListToolStripMenuItem.Click += new System.EventHandler(this.keyword_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
            this.aboutToolStripMenuItem.Text = "&About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.about_Click);
            // 
            // panel1
            // 
            this.panel1.BackColor = System.Drawing.SystemColors.Window;
            this.panel1.Controls.Add(this.refreshListBox);
            this.panel1.Controls.Add(this.listBoxForParse);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Location = new System.Drawing.Point(12, 27);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(507, 56);
            this.panel1.TabIndex = 10;
            // 
            // refreshListBox
            // 
            this.refreshListBox.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.refreshListBox.Location = new System.Drawing.Point(292, 4);
            this.refreshListBox.Name = "refreshListBox";
            this.refreshListBox.Size = new System.Drawing.Size(75, 23);
            this.refreshListBox.TabIndex = 2;
            this.refreshListBox.Text = "Refresh Lists";
            this.logParserTooltip.SetToolTip(this.refreshListBox, "Click to Refresh the Keyword List Box if you are not seeing New or still seeing D" +
                    "eleted Keyword Lists after using the Keyword Editor");
            this.refreshListBox.UseVisualStyleBackColor = false;
            this.refreshListBox.Click += new System.EventHandler(this.refreshListBox_Click);
            // 
            // listBoxForParse
            // 
            this.listBoxForParse.FormattingEnabled = true;
            this.listBoxForParse.Location = new System.Drawing.Point(120, 4);
            this.listBoxForParse.Name = "listBoxForParse";
            this.listBoxForParse.Size = new System.Drawing.Size(165, 43);
            this.listBoxForParse.TabIndex = 1;
            this.logParserTooltip.SetToolTip(this.listBoxForParse, "Keyword Lists");
            this.listBoxForParse.SelectedIndexChanged += new System.EventHandler(this.listBoxForParse_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(4, 4);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(118, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Keyword List for Parse: ";
            // 
            // closeProgram
            // 
            this.closeProgram.Location = new System.Drawing.Point(444, 373);
            this.closeProgram.Name = "closeProgram";
            this.closeProgram.Size = new System.Drawing.Size(75, 23);
            this.closeProgram.TabIndex = 11;
            this.closeProgram.Text = "Close";
            this.logParserTooltip.SetToolTip(this.closeProgram, "Close the Program");
            this.closeProgram.UseVisualStyleBackColor = true;
            this.closeProgram.Click += new System.EventHandler(this.closeProgram_Click);
            // 
            // parseButton
            // 
            this.parseButton.Location = new System.Drawing.Point(12, 373);
            this.parseButton.Name = "parseButton";
            this.parseButton.Size = new System.Drawing.Size(75, 23);
            this.parseButton.TabIndex = 12;
            this.parseButton.Text = "Parse!";
            this.logParserTooltip.SetToolTip(this.parseButton, "Click here for the Open a new Log File and begin parsing");
            this.parseButton.UseVisualStyleBackColor = true;
            this.parseButton.Click += new System.EventHandler(this.parseButton_Click);
            // 
            // parseResultsBox
            // 
            this.parseResultsBox.BackColor = System.Drawing.SystemColors.Window;
            this.parseResultsBox.Location = new System.Drawing.Point(12, 91);
            this.parseResultsBox.Multiline = true;
            this.parseResultsBox.Name = "parseResultsBox";
            this.parseResultsBox.ReadOnly = true;
            this.parseResultsBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.parseResultsBox.Size = new System.Drawing.Size(507, 277);
            this.parseResultsBox.TabIndex = 13;
            this.logParserTooltip.SetToolTip(this.parseResultsBox, "Parse Results Display Box");
            this.parseResultsBox.WordWrap = false;
            // 
            // saveButton
            // 
            this.saveButton.Enabled = false;
            this.saveButton.Location = new System.Drawing.Point(363, 373);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(75, 23);
            this.saveButton.TabIndex = 14;
            this.saveButton.Text = "Save";
            this.logParserTooltip.SetToolTip(this.saveButton, "Saves the current Parse Results as Text Log or XML");
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // POLLogParserForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.ControlLight;
            this.ClientSize = new System.Drawing.Size(531, 408);
            this.Controls.Add(this.saveButton);
            this.Controls.Add(this.parseResultsBox);
            this.Controls.Add(this.parseButton);
            this.Controls.Add(this.closeProgram);
            this.Controls.Add(this.menuStrip1);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "POLLogParserForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "POL Log Parser";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem settingsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem keywordListToolStripMenuItem;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button closeProgram;
        private System.Windows.Forms.Button parseButton;
        private System.Windows.Forms.TextBox parseResultsBox;
        private System.Windows.Forms.Button saveButton;
        private System.Windows.Forms.ToolStripMenuItem optionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pOLLogFilesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem scriptslogToolStripMenuItem;
        private System.Windows.Forms.ListBox listBoxForParse;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button refreshListBox;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem debuglogToolStripMenuItem;
        private System.Windows.Forms.ToolTip logParserTooltip;

    }
}

