using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;

namespace ProjectOverboss
{
    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new MainForm());
        }
    }

    public class MainForm : Form
    {
        private Label lblTitle = null!;
        private Label lblStatus = null!;
        private Label lblHotkeys = null!;
        private Button btnLaunchGame = null!;
        private Button btnInject = null!;
        private Button btnOpenFolder = null!;
        private Button btnOpenGithub = null!;
        private System.Windows.Forms.Timer statusTimer = null!;

        private const string GameProcessName = "Fallout4";
        private const string SteamLaunchUri = "steam://rungameid/377160";
        private const string GithubUrl = "https://github.com/codisfun420-hub/project-overboss";
        private readonly string ModDirectory;

        // P/Invoke for remote DLL injection
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

        [DllImport("kernel32.dll", SetLastError = true, ExactSpelling = true)]
        private static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out UIntPtr lpNumberOfBytesWritten);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, out IntPtr lpThreadId);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool CloseHandle(IntPtr hObject);

        private const uint PROCESS_CREATE_THREAD = 0x0002;
        private const uint PROCESS_QUERY_INFORMATION = 0x0400;
        private const uint PROCESS_VM_OPERATION = 0x0008;
        private const uint PROCESS_VM_WRITE = 0x0020;
        private const uint PROCESS_VM_READ = 0x0010;
        private const uint MEM_COMMIT = 0x00001000;
        private const uint MEM_RESERVE = 0x00002000;
        private const uint PAGE_READWRITE = 0x04;

        public MainForm()
        {
            ModDirectory = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), @".gemini\antigravity\scratch\project-overboss");

            InitializeComponent();
            ApplyPipBoyStyling();

            statusTimer = new System.Windows.Forms.Timer { Interval = 1500 };
            statusTimer.Tick += (s, e) => UpdateGameStatus();
            statusTimer.Start();
            UpdateGameStatus();
        }

        private void InitializeComponent()
        {
            this.Text = "Project Overboss // Launcher & Injector";
            this.Size = new Size(520, 480);
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;

            if (File.Exists("logo.ico"))
            {
                this.Icon = new Icon("logo.ico");
            }

            lblTitle = new Label
            {
                Text = "PROJECT OVERBOSS",
                Font = new Font("Consolas", 18, FontStyle.Bold),
                TextAlign = ContentAlignment.MiddleCenter,
                Location = new Point(20, 20),
                Size = new Size(465, 35)
            };

            lblStatus = new Label
            {
                Text = "Game Status: Scanning...",
                Font = new Font("Consolas", 11, FontStyle.Regular),
                TextAlign = ContentAlignment.MiddleCenter,
                Location = new Point(20, 65),
                Size = new Size(465, 30)
            };

            btnLaunchGame = new Button
            {
                Text = "Launch Fallout 4 (Steam)",
                Font = new Font("Consolas", 11, FontStyle.Bold),
                Location = new Point(60, 115),
                Size = new Size(385, 45),
                FlatStyle = FlatStyle.Flat
            };
            btnLaunchGame.Click += (s, e) => LaunchGame();

            btnInject = new Button
            {
                Text = "Inject Project Overboss DLL",
                Font = new Font("Consolas", 11, FontStyle.Bold),
                Location = new Point(60, 175),
                Size = new Size(385, 45),
                FlatStyle = FlatStyle.Flat
            };
            btnInject.Click += (s, e) => InjectDll();

            btnOpenFolder = new Button
            {
                Text = "Open Mod Source Folder",
                Font = new Font("Consolas", 10, FontStyle.Regular),
                Location = new Point(60, 235),
                Size = new Size(185, 38),
                FlatStyle = FlatStyle.Flat
            };
            btnOpenFolder.Click += (s, e) => Process.Start(new ProcessStartInfo("explorer.exe", ModDirectory));

            btnOpenGithub = new Button
            {
                Text = "GitHub Repository",
                Font = new Font("Consolas", 10, FontStyle.Regular),
                Location = new Point(260, 235),
                Size = new Size(185, 38),
                FlatStyle = FlatStyle.Flat
            };
            btnOpenGithub.Click += (s, e) => Process.Start(new ProcessStartInfo { FileName = GithubUrl, UseShellExecute = true });

            lblHotkeys = new Label
            {
                Text = "Pip-Boy In-Game Hotkeys:\n[INSERT] : Toggle Mod Menu Overlay\n[END]    : Emergency Panic Eject & Unhook",
                Font = new Font("Consolas", 9.5f, FontStyle.Regular),
                Location = new Point(60, 300),
                Size = new Size(385, 90),
                BorderStyle = BorderStyle.FixedSingle,
                TextAlign = ContentAlignment.MiddleCenter
            };

            this.Controls.Add(lblTitle);
            this.Controls.Add(lblStatus);
            this.Controls.Add(btnLaunchGame);
            this.Controls.Add(btnInject);
            this.Controls.Add(btnOpenFolder);
            this.Controls.Add(btnOpenGithub);
            this.Controls.Add(lblHotkeys);
        }

        private void ApplyPipBoyStyling()
        {
            Color darkBg = Color.FromArgb(13, 14, 18);
            Color neonGreen = Color.FromArgb(51, 242, 89);
            Color buttonBg = Color.FromArgb(20, 35, 22);

            this.BackColor = darkBg;
            lblTitle.ForeColor = neonGreen;
            lblStatus.ForeColor = Color.FromArgb(180, 255, 190);
            lblHotkeys.ForeColor = neonGreen;
            lblHotkeys.BackColor = Color.FromArgb(16, 20, 18);

            ApplyButtonTheme(btnLaunchGame, buttonBg, neonGreen);
            ApplyButtonTheme(btnInject, buttonBg, neonGreen);
            ApplyButtonTheme(btnOpenFolder, buttonBg, neonGreen);
            ApplyButtonTheme(btnOpenGithub, buttonBg, neonGreen);
        }

        private void ApplyButtonTheme(Button btn, Color bg, Color text)
        {
            btn.BackColor = bg;
            btn.ForeColor = text;
            btn.FlatAppearance.BorderColor = text;
            btn.FlatAppearance.BorderSize = 1;
            btn.Cursor = Cursors.Hand;
        }

        private void UpdateGameStatus()
        {
            var processes = Process.GetProcessesByName(GameProcessName);
            if (processes.Length > 0)
            {
                lblStatus.Text = $"Fallout 4 Detected (PID: {processes[0].Id})";
                lblStatus.ForeColor = Color.FromArgb(51, 242, 89);
                btnInject.Enabled = true;
            }
            else
            {
                lblStatus.Text = "Game Status: Fallout 4 is NOT running";
                lblStatus.ForeColor = Color.FromArgb(200, 100, 100);
            }
        }

        private void LaunchGame()
        {
            try
            {
                Process.Start(new ProcessStartInfo { FileName = SteamLaunchUri, UseShellExecute = true });
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Could not launch game via Steam: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void InjectDll()
        {
            var processes = Process.GetProcessesByName(GameProcessName);
            if (processes.Length == 0)
            {
                MessageBox.Show("Fallout 4 is not running. Please launch the game first.", "Process Not Found", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            string localDll = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "ProjectOverboss.dll");
            string dllPath = localDll;
            if (!File.Exists(dllPath))
            {
                dllPath = Path.Combine(ModDirectory, "ProjectOverboss.dll");
            }
            if (!File.Exists(dllPath))
            {
                dllPath = Path.Combine(ModDirectory, @"build\Release\ProjectOverboss.dll");
            }

            if (!File.Exists(dllPath))
            {
                MessageBox.Show($"ProjectOverboss.dll not found!\nExpected at:\n{dllPath}\n\nPlease compile the C++ DLL using CMake / Visual Studio first.", "DLL Not Found", MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            int targetPid = processes[0].Id;
            IntPtr hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, targetPid);
            if (hProcess == IntPtr.Zero)
            {
                MessageBox.Show("Failed to open Fallout 4 process. Try running this launcher as Administrator.", "Permission Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                byte[] bytes = Encoding.ASCII.GetBytes(dllPath + "\0");
                IntPtr allocMemAddress = VirtualAllocEx(hProcess, IntPtr.Zero, (uint)bytes.Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                if (allocMemAddress == IntPtr.Zero)
                {
                    MessageBox.Show("Failed to allocate memory in target process.", "Allocation Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                WriteProcessMemory(hProcess, allocMemAddress, bytes, (uint)bytes.Length, out _);

                IntPtr loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
                IntPtr hThread = CreateRemoteThread(hProcess, IntPtr.Zero, 0, loadLibraryAddr, allocMemAddress, 0, out _);

                if (hThread != IntPtr.Zero)
                {
                    CloseHandle(hThread);
                    MessageBox.Show("Project Overboss injected successfully!\n\nSwitch to Fallout 4 and press [INSERT] to open the menu.", "Injection Complete", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                    MessageBox.Show("CreateRemoteThread failed.", "Injection Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
            finally
            {
                CloseHandle(hProcess);
            }
        }
    }
}
