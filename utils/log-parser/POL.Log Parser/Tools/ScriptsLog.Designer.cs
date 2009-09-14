namespace POL.Log_Parser.Tools
{
    partial class ScriptsLogForm
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
            this.textPanel = new System.Windows.Forms.Panel();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.parseResultsTextBox = new System.Windows.Forms.TextBox();
            this.closeButton = new System.Windows.Forms.Button();
            this.saveButton = new System.Windows.Forms.Button();
            this.parseButton = new System.Windows.Forms.Button();
            this.scriptLogTooltip = new System.Windows.Forms.ToolTip(this.components);
            this.textPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // textPanel
            // 
            this.textPanel.BackColor = System.Drawing.SystemColors.Window;
            this.textPanel.Controls.Add(this.label3);
            this.textPanel.Controls.Add(this.label2);
            this.textPanel.Controls.Add(this.label1);
            this.textPanel.Location = new System.Drawing.Point(12, 12);
            this.textPanel.Name = "textPanel";
            this.textPanel.Size = new System.Drawing.Size(507, 56);
            this.textPanel.TabIndex = 11;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(64, 34);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(379, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "If you wish to keep the results, be sure to click the Save button before you exit" +
                "!";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(62, 21);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(383, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "you press the Parse Button. Results are individual Instances of runaway scripts. " +
                "";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(32, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(443, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Welcome to the POL Scripts.Log Parser!   All  results will be displayed in the bo" +
                "x below once ";
            // 
            // parseResultsTextBox
            // 
            this.parseResultsTextBox.BackColor = System.Drawing.SystemColors.Window;
            this.parseResultsTextBox.Location = new System.Drawing.Point(12, 90);
            this.parseResultsTextBox.Multiline = true;
            this.parseResultsTextBox.Name = "parseResultsTextBox";
            this.parseResultsTextBox.ReadOnly = true;
            this.parseResultsTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.parseResultsTextBox.Size = new System.Drawing.Size(507, 239);
            this.parseResultsTextBox.TabIndex = 14;
            this.scriptLogTooltip.SetToolTip(this.parseResultsTextBox, "Parse Results Display Box");
            this.parseResultsTextBox.WordWrap = false;
            // 
            // closeButton
            // 
            this.closeButton.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.closeButton.Location = new System.Drawing.Point(444, 345);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(75, 23);
            this.closeButton.TabIndex = 15;
            this.closeButton.Text = "Close";
            this.closeButton.UseVisualStyleBackColor = false;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // saveButton
            // 
            this.saveButton.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.saveButton.Enabled = false;
            this.saveButton.Location = new System.Drawing.Point(363, 345);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(75, 23);
            this.saveButton.TabIndex = 16;
            this.saveButton.Text = "Save";
            this.scriptLogTooltip.SetToolTip(this.saveButton, "Saves the current Parse Results as Text Log or XML");
            this.saveButton.UseVisualStyleBackColor = false;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // parseButton
            // 
            this.parseButton.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.parseButton.Location = new System.Drawing.Point(12, 345);
            this.parseButton.Name = "parseButton";
            this.parseButton.Size = new System.Drawing.Size(75, 23);
            this.parseButton.TabIndex = 17;
            this.parseButton.Text = "Parse";
            this.scriptLogTooltip.SetToolTip(this.parseButton, "Click here for the Open Script.Log File dialog and to begin parsing");
            this.parseButton.UseVisualStyleBackColor = false;
            this.parseButton.Click += new System.EventHandler(this.parseButton_Click);
            // 
            // ScriptsLogForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(531, 380);
            this.ControlBox = false;
            this.Controls.Add(this.parseButton);
            this.Controls.Add(this.saveButton);
            this.Controls.Add(this.closeButton);
            this.Controls.Add(this.parseResultsTextBox);
            this.Controls.Add(this.textPanel);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ScriptsLogForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "POL Scripts.Log Parser";
            this.textPanel.ResumeLayout(false);
            this.textPanel.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel textPanel;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox parseResultsTextBox;
        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.Button saveButton;
        private System.Windows.Forms.Button parseButton;
        private System.Windows.Forms.ToolTip scriptLogTooltip;
    }
}