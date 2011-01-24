namespace POL.Log_Parser
{
    partial class OptionsForm
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
            this.defaultLogPathGroupBox = new System.Windows.Forms.GroupBox();
            this.browseButton = new System.Windows.Forms.Button();
            this.defaultButton = new System.Windows.Forms.Button();
            this.defaultLogPathTextBox = new System.Windows.Forms.TextBox();
            this.defaultOutputPathGroupBox = new System.Windows.Forms.GroupBox();
            this.browseOutputPathButton = new System.Windows.Forms.Button();
            this.defaultOutputPathButton = new System.Windows.Forms.Button();
            this.defaultOutputPathBox = new System.Windows.Forms.TextBox();
            this.saveAllButton = new System.Windows.Forms.Button();
            this.resetButton = new System.Windows.Forms.Button();
            this.defaultToolsPath = new System.Windows.Forms.GroupBox();
            this.browseToolsPathButton = new System.Windows.Forms.Button();
            this.defaultToolsPathButton = new System.Windows.Forms.Button();
            this.defaultToolsPathTextBox = new System.Windows.Forms.TextBox();
            this.caseSensitiveParseCheckbox = new System.Windows.Forms.CheckBox();
            this.optionsPageTooltip = new System.Windows.Forms.ToolTip(this.components);
            this.defaultLogPathGroupBox.SuspendLayout();
            this.defaultOutputPathGroupBox.SuspendLayout();
            this.defaultToolsPath.SuspendLayout();
            this.SuspendLayout();
            // 
            // defaultLogPathGroupBox
            // 
            this.defaultLogPathGroupBox.Controls.Add(this.browseButton);
            this.defaultLogPathGroupBox.Controls.Add(this.defaultButton);
            this.defaultLogPathGroupBox.Controls.Add(this.defaultLogPathTextBox);
            this.defaultLogPathGroupBox.Location = new System.Drawing.Point(12, 12);
            this.defaultLogPathGroupBox.Name = "defaultLogPathGroupBox";
            this.defaultLogPathGroupBox.Size = new System.Drawing.Size(351, 85);
            this.defaultLogPathGroupBox.TabIndex = 4;
            this.defaultLogPathGroupBox.TabStop = false;
            this.defaultLogPathGroupBox.Text = "Default Log Folder";
            this.optionsPageTooltip.SetToolTip(this.defaultLogPathGroupBox, "This is the location when you click on Parse in the Main Log Parser Window. ");
            // 
            // browseButton
            // 
            this.browseButton.Location = new System.Drawing.Point(87, 56);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(75, 23);
            this.browseButton.TabIndex = 2;
            this.browseButton.Text = "Browse";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // defaultButton
            // 
            this.defaultButton.Location = new System.Drawing.Point(6, 56);
            this.defaultButton.Name = "defaultButton";
            this.defaultButton.Size = new System.Drawing.Size(75, 23);
            this.defaultButton.TabIndex = 1;
            this.defaultButton.Text = "Default";
            this.defaultButton.UseVisualStyleBackColor = true;
            this.defaultButton.Click += new System.EventHandler(this.defaultButton_Click);
            // 
            // defaultLogPathTextBox
            // 
            this.defaultLogPathTextBox.Location = new System.Drawing.Point(6, 30);
            this.defaultLogPathTextBox.Name = "defaultLogPathTextBox";
            this.defaultLogPathTextBox.ReadOnly = true;
            this.defaultLogPathTextBox.Size = new System.Drawing.Size(338, 20);
            this.defaultLogPathTextBox.TabIndex = 0;
            // 
            // defaultOutputPathGroupBox
            // 
            this.defaultOutputPathGroupBox.Controls.Add(this.browseOutputPathButton);
            this.defaultOutputPathGroupBox.Controls.Add(this.defaultOutputPathButton);
            this.defaultOutputPathGroupBox.Controls.Add(this.defaultOutputPathBox);
            this.defaultOutputPathGroupBox.Location = new System.Drawing.Point(12, 103);
            this.defaultOutputPathGroupBox.Name = "defaultOutputPathGroupBox";
            this.defaultOutputPathGroupBox.Size = new System.Drawing.Size(351, 85);
            this.defaultOutputPathGroupBox.TabIndex = 5;
            this.defaultOutputPathGroupBox.TabStop = false;
            this.defaultOutputPathGroupBox.Text = "Default Output Folder";
            this.optionsPageTooltip.SetToolTip(this.defaultOutputPathGroupBox, "This is the location when you click on Save in the Main Log Parser Window. ");
            // 
            // browseOutputPathButton
            // 
            this.browseOutputPathButton.Location = new System.Drawing.Point(87, 56);
            this.browseOutputPathButton.Name = "browseOutputPathButton";
            this.browseOutputPathButton.Size = new System.Drawing.Size(75, 23);
            this.browseOutputPathButton.TabIndex = 2;
            this.browseOutputPathButton.Text = "Browse";
            this.browseOutputPathButton.UseVisualStyleBackColor = true;
            this.browseOutputPathButton.Click += new System.EventHandler(this.browseOutputPathButton_Click);
            // 
            // defaultOutputPathButton
            // 
            this.defaultOutputPathButton.Location = new System.Drawing.Point(6, 56);
            this.defaultOutputPathButton.Name = "defaultOutputPathButton";
            this.defaultOutputPathButton.Size = new System.Drawing.Size(75, 23);
            this.defaultOutputPathButton.TabIndex = 1;
            this.defaultOutputPathButton.Text = "Default";
            this.defaultOutputPathButton.UseVisualStyleBackColor = true;
            this.defaultOutputPathButton.Click += new System.EventHandler(this.defaultOutputPathButton_Click);
            // 
            // defaultOutputPathBox
            // 
            this.defaultOutputPathBox.Location = new System.Drawing.Point(6, 30);
            this.defaultOutputPathBox.Name = "defaultOutputPathBox";
            this.defaultOutputPathBox.ReadOnly = true;
            this.defaultOutputPathBox.Size = new System.Drawing.Size(338, 20);
            this.defaultOutputPathBox.TabIndex = 0;
            // 
            // saveAllButton
            // 
            this.saveAllButton.Location = new System.Drawing.Point(446, 285);
            this.saveAllButton.Name = "saveAllButton";
            this.saveAllButton.Size = new System.Drawing.Size(75, 23);
            this.saveAllButton.TabIndex = 6;
            this.saveAllButton.Text = "OK";
            this.saveAllButton.UseVisualStyleBackColor = true;
            this.saveAllButton.Click += new System.EventHandler(this.saveAllButton_Click);
            // 
            // resetButton
            // 
            this.resetButton.Location = new System.Drawing.Point(365, 285);
            this.resetButton.Name = "resetButton";
            this.resetButton.Size = new System.Drawing.Size(75, 23);
            this.resetButton.TabIndex = 7;
            this.resetButton.Text = "Reset";
            this.resetButton.UseVisualStyleBackColor = true;
            this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
            // 
            // defaultToolsPath
            // 
            this.defaultToolsPath.Controls.Add(this.browseToolsPathButton);
            this.defaultToolsPath.Controls.Add(this.defaultToolsPathButton);
            this.defaultToolsPath.Controls.Add(this.defaultToolsPathTextBox);
            this.defaultToolsPath.Location = new System.Drawing.Point(12, 194);
            this.defaultToolsPath.Name = "defaultToolsPath";
            this.defaultToolsPath.Size = new System.Drawing.Size(351, 85);
            this.defaultToolsPath.TabIndex = 8;
            this.defaultToolsPath.TabStop = false;
            this.defaultToolsPath.Text = "Default Tools Path (POL)";
            this.optionsPageTooltip.SetToolTip(this.defaultToolsPath, "This is the location when click on Parse in any POL Tools Log Parser");
            // 
            // browseToolsPathButton
            // 
            this.browseToolsPathButton.Location = new System.Drawing.Point(87, 56);
            this.browseToolsPathButton.Name = "browseToolsPathButton";
            this.browseToolsPathButton.Size = new System.Drawing.Size(75, 23);
            this.browseToolsPathButton.TabIndex = 3;
            this.browseToolsPathButton.Text = "Browse";
            this.browseToolsPathButton.UseVisualStyleBackColor = true;
            this.browseToolsPathButton.Click += new System.EventHandler(this.browseToolsPathButton_Click);
            // 
            // defaultToolsPathButton
            // 
            this.defaultToolsPathButton.Location = new System.Drawing.Point(6, 56);
            this.defaultToolsPathButton.Name = "defaultToolsPathButton";
            this.defaultToolsPathButton.Size = new System.Drawing.Size(75, 23);
            this.defaultToolsPathButton.TabIndex = 2;
            this.defaultToolsPathButton.Text = "Default";
            this.defaultToolsPathButton.UseVisualStyleBackColor = true;
            this.defaultToolsPathButton.Click += new System.EventHandler(this.defaultToolsPathButton_Click);
            // 
            // defaultToolsPathTextBox
            // 
            this.defaultToolsPathTextBox.Location = new System.Drawing.Point(6, 30);
            this.defaultToolsPathTextBox.Name = "defaultToolsPathTextBox";
            this.defaultToolsPathTextBox.ReadOnly = true;
            this.defaultToolsPathTextBox.Size = new System.Drawing.Size(338, 20);
            this.defaultToolsPathTextBox.TabIndex = 1;
            // 
            // caseSensitiveParseCheckbox
            // 
            this.caseSensitiveParseCheckbox.AutoSize = true;
            this.caseSensitiveParseCheckbox.Location = new System.Drawing.Point(386, 44);
            this.caseSensitiveParseCheckbox.Name = "caseSensitiveParseCheckbox";
            this.caseSensitiveParseCheckbox.Size = new System.Drawing.Size(126, 17);
            this.caseSensitiveParseCheckbox.TabIndex = 9;
            this.caseSensitiveParseCheckbox.Text = "Case Sensitive Parse";
            this.optionsPageTooltip.SetToolTip(this.caseSensitiveParseCheckbox, "Mark this box to use Case Sensitive Parsing based on Keyword (Ignore Case)");
            this.caseSensitiveParseCheckbox.UseVisualStyleBackColor = true;
            // 
            // OptionsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(533, 315);
            this.ControlBox = false;
            this.Controls.Add(this.caseSensitiveParseCheckbox);
            this.Controls.Add(this.defaultToolsPath);
            this.Controls.Add(this.resetButton);
            this.Controls.Add(this.saveAllButton);
            this.Controls.Add(this.defaultOutputPathGroupBox);
            this.Controls.Add(this.defaultLogPathGroupBox);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "OptionsForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Options";
            this.defaultLogPathGroupBox.ResumeLayout(false);
            this.defaultLogPathGroupBox.PerformLayout();
            this.defaultOutputPathGroupBox.ResumeLayout(false);
            this.defaultOutputPathGroupBox.PerformLayout();
            this.defaultToolsPath.ResumeLayout(false);
            this.defaultToolsPath.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox defaultLogPathGroupBox;
        private System.Windows.Forms.TextBox defaultLogPathTextBox;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.Button defaultButton;
        private System.Windows.Forms.GroupBox defaultOutputPathGroupBox;
        private System.Windows.Forms.Button browseOutputPathButton;
        private System.Windows.Forms.Button defaultOutputPathButton;
        private System.Windows.Forms.TextBox defaultOutputPathBox;
        private System.Windows.Forms.Button saveAllButton;
        private System.Windows.Forms.Button resetButton;
        private System.Windows.Forms.GroupBox defaultToolsPath;
        private System.Windows.Forms.Button browseToolsPathButton;
        private System.Windows.Forms.Button defaultToolsPathButton;
        private System.Windows.Forms.TextBox defaultToolsPathTextBox;
        private System.Windows.Forms.CheckBox caseSensitiveParseCheckbox;
        private System.Windows.Forms.ToolTip optionsPageTooltip;
    }
}