namespace POL.Log_Parser
{
    partial class KeywordForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(KeywordForm));
            this.KeywordGrid = new System.Windows.Forms.DataGridView();
            this.Keywords = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.List_Name = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.closeButton = new System.Windows.Forms.Button();
            this.resetButton = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.saveButton = new System.Windows.Forms.Button();
            this.keywordListsBox = new System.Windows.Forms.ListBox();
            this.loadListButton = new System.Windows.Forms.Button();
            this.newListButton = new System.Windows.Forms.Button();
            this.deleteListButton = new System.Windows.Forms.Button();
            this.newListNameTextBox = new System.Windows.Forms.TextBox();
            this.keywordEditorTooltip = new System.Windows.Forms.ToolTip(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.KeywordGrid)).BeginInit();
            this.SuspendLayout();
            // 
            // KeywordGrid
            // 
            this.KeywordGrid.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.KeywordGrid.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.Keywords,
            this.List_Name});
            this.KeywordGrid.Location = new System.Drawing.Point(12, 12);
            this.KeywordGrid.Name = "KeywordGrid";
            this.KeywordGrid.Size = new System.Drawing.Size(193, 337);
            this.KeywordGrid.TabIndex = 0;
            this.keywordEditorTooltip.SetToolTip(this.KeywordGrid, "Keywords for the currently loaded Keyword List in the Keyword Lists Box on the ri" +
                    "ght");
            this.KeywordGrid.UserAddedRow += new System.Windows.Forms.DataGridViewRowEventHandler(this.KeywordGrid_UserAddedRow);
            // 
            // Keywords
            // 
            this.Keywords.HeaderText = "Keywords";
            this.Keywords.MaxInputLength = 100;
            this.Keywords.Name = "Keywords";
            this.Keywords.ToolTipText = "Enter the Keywords you wish to check for in POL.Log";
            this.Keywords.Width = 150;
            // 
            // List_Name
            // 
            this.List_Name.HeaderText = "List Name";
            this.List_Name.MaxInputLength = 50;
            this.List_Name.Name = "List_Name";
            this.List_Name.ReadOnly = true;
            this.List_Name.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.List_Name.Visible = false;
            // 
            // closeButton
            // 
            this.closeButton.Location = new System.Drawing.Point(293, 326);
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size(75, 23);
            this.closeButton.TabIndex = 1;
            this.closeButton.Text = "&Close";
            this.keywordEditorTooltip.SetToolTip(this.closeButton, "Closes the Keyword Editor without saving any changed data");
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler(this.closeButton_Click);
            // 
            // resetButton
            // 
            this.resetButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.resetButton.Location = new System.Drawing.Point(212, 326);
            this.resetButton.Name = "resetButton";
            this.resetButton.Size = new System.Drawing.Size(75, 23);
            this.resetButton.TabIndex = 2;
            this.resetButton.Text = "Clear";
            this.keywordEditorTooltip.SetToolTip(this.resetButton, "Clear the current Keyword List in the Editor");
            this.resetButton.UseVisualStyleBackColor = true;
            this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
            // 
            // textBox1
            // 
            this.textBox1.BackColor = System.Drawing.SystemColors.Info;
            this.textBox1.Location = new System.Drawing.Point(212, 13);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(165, 115);
            this.textBox1.TabIndex = 3;
            this.textBox1.Text = "Enter Keywords that you wish to scan for. Enter a single keyword or phrase per li" +
                "ne.\r\n\r\nTo clear the entire list, click Clear. Once finished click Save, then Clo" +
                "se to close this window.";
            // 
            // saveButton
            // 
            this.saveButton.Location = new System.Drawing.Point(293, 297);
            this.saveButton.Name = "saveButton";
            this.saveButton.Size = new System.Drawing.Size(75, 23);
            this.saveButton.TabIndex = 4;
            this.saveButton.Text = "Save Lists";
            this.keywordEditorTooltip.SetToolTip(this.saveButton, "Saves all Keyword Lists");
            this.saveButton.UseVisualStyleBackColor = true;
            this.saveButton.Click += new System.EventHandler(this.saveButton_Click);
            // 
            // keywordListsBox
            // 
            this.keywordListsBox.FormattingEnabled = true;
            this.keywordListsBox.Location = new System.Drawing.Point(212, 134);
            this.keywordListsBox.Name = "keywordListsBox";
            this.keywordListsBox.Size = new System.Drawing.Size(165, 95);
            this.keywordListsBox.Sorted = true;
            this.keywordListsBox.TabIndex = 5;
            this.keywordEditorTooltip.SetToolTip(this.keywordListsBox, "Keyword Lists Selection Box");
            // 
            // loadListButton
            // 
            this.loadListButton.Location = new System.Drawing.Point(212, 297);
            this.loadListButton.Name = "loadListButton";
            this.loadListButton.Size = new System.Drawing.Size(75, 23);
            this.loadListButton.TabIndex = 6;
            this.loadListButton.Text = "Load List";
            this.keywordEditorTooltip.SetToolTip(this.loadListButton, "Loads the currently highlighted Keyword List in the Keyword Lists Selection Box");
            this.loadListButton.UseVisualStyleBackColor = true;
            this.loadListButton.Click += new System.EventHandler(this.loadListButton_Click);
            // 
            // newListButton
            // 
            this.newListButton.Location = new System.Drawing.Point(212, 268);
            this.newListButton.Name = "newListButton";
            this.newListButton.Size = new System.Drawing.Size(75, 23);
            this.newListButton.TabIndex = 7;
            this.newListButton.Text = "New List";
            this.keywordEditorTooltip.SetToolTip(this.newListButton, "Will create a new Keyword List in the Editor using the name in the above box as i" +
                    "t\'s List Name");
            this.newListButton.UseVisualStyleBackColor = true;
            this.newListButton.Click += new System.EventHandler(this.newListButton_Click);
            // 
            // deleteListButton
            // 
            this.deleteListButton.Location = new System.Drawing.Point(293, 268);
            this.deleteListButton.Name = "deleteListButton";
            this.deleteListButton.Size = new System.Drawing.Size(75, 23);
            this.deleteListButton.TabIndex = 8;
            this.deleteListButton.Text = "Delete List";
            this.keywordEditorTooltip.SetToolTip(this.deleteListButton, "Delete the currently highlighted Keyword List in the Keyword Lists Selection Box");
            this.deleteListButton.UseVisualStyleBackColor = true;
            this.deleteListButton.Click += new System.EventHandler(this.deleteListButton_Click);
            // 
            // newListNameTextBox
            // 
            this.newListNameTextBox.Location = new System.Drawing.Point(212, 236);
            this.newListNameTextBox.MaxLength = 50;
            this.newListNameTextBox.Name = "newListNameTextBox";
            this.newListNameTextBox.Size = new System.Drawing.Size(165, 20);
            this.newListNameTextBox.TabIndex = 9;
            this.newListNameTextBox.Text = "New List Name Here";
            this.keywordEditorTooltip.SetToolTip(this.newListNameTextBox, "Type in a Name to create a new Keyword List and click on the New List Button");
            // 
            // KeywordForm
            // 
            this.AcceptButton = this.saveButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.CancelButton = this.resetButton;
            this.ClientSize = new System.Drawing.Size(380, 361);
            this.ControlBox = false;
            this.Controls.Add(this.newListNameTextBox);
            this.Controls.Add(this.deleteListButton);
            this.Controls.Add(this.newListButton);
            this.Controls.Add(this.loadListButton);
            this.Controls.Add(this.keywordListsBox);
            this.Controls.Add(this.saveButton);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.resetButton);
            this.Controls.Add(this.closeButton);
            this.Controls.Add(this.KeywordGrid);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.MinimizeBox = false;
            this.Name = "KeywordForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Keyword List";
            ((System.ComponentModel.ISupportInitialize)(this.KeywordGrid)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView KeywordGrid;
        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.Button resetButton;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Button saveButton;
        private System.Windows.Forms.ListBox keywordListsBox;
        private System.Windows.Forms.Button loadListButton;
        private System.Windows.Forms.Button newListButton;
        private System.Windows.Forms.Button deleteListButton;
        private System.Windows.Forms.DataGridViewTextBoxColumn Keywords;
        private System.Windows.Forms.DataGridViewTextBoxColumn List_Name;
        private System.Windows.Forms.TextBox newListNameTextBox;
        private System.Windows.Forms.ToolTip keywordEditorTooltip;
    }
}