// TigerScale software connector -- the PC side of the parcel scale (/colis).
//
// A tray program that serves the scale's /colis page at http://127.0.0.1:47800/
// and relays its two requests (/api/parcel, /api/tare) to the scale, so the
// page has one origin and can ask this program to type the weight into another
// application -- something no web page may do on its own. It is optional: the
// scale's own /colis page works without it, with Copy only.
//
// Why the page is served from here rather than calling here from the scale's
// address: Chrome and Edge refuse a plain-http page on the local network any
// request to loopback (Local Network Access is limited to secure contexts), and
// the scale serves no https. Loopback to loopback is not restricted.
//
// What it is allowed to do, deliberately narrow (README.md has the reasons):
//   - listen on 127.0.0.1 only, and answer only requests whose Host is its own
//     (a DNS-rebinding page cannot reach it under another name);
//   - accept a POST only from its own page's Origin (another site's form or
//     fetch carries that site's Origin and is refused);
//   - type nothing but a short weight: digits, one separator, an optional unit.
//
// Written for the C# 5 compiler that ships inside every Windows 10/11
// (.NET Framework 4.x), so build.cmd needs nothing installed. Keep it that way:
// no string interpolation, no ?., no expression-bodied members.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using System.Windows.Forms;
using Microsoft.Win32;

[assembly: System.Reflection.AssemblyTitle("TigerScale Connector")]
[assembly: System.Reflection.AssemblyProduct("TigerScale Connector")]
[assembly: System.Reflection.AssemblyVersion("1.0.0.0")]

namespace TigerScaleConnector
{
    static class Program
    {
        public const string Version = "1.0.0";
        public const int Port = 47800;

        [STAThread]
        static void Main()
        {
            bool first;
            using (Mutex m = new Mutex(true, "TigerScaleConnector.SingleInstance", out first))
            {
                if (!first)
                {
                    // Already running: just bring its page up again.
                    Browser.Open("http://127.0.0.1:" + Port + "/");
                    return;
                }
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new Tray());
            }
        }
    }

    // ---------------------------------------------------------------------
    // Texts. French when Windows is in French, English otherwise.
    // ---------------------------------------------------------------------
    static class T
    {
        static readonly bool Fr = System.Globalization.CultureInfo.CurrentUICulture
                                      .TwoLetterISOLanguageName == "fr";
        public static string Open      { get { return Fr ? "Ouvrir le pèse-colis" : "Open the parcel scale"; } }
        public static string Startup   { get { return Fr ? "Lancer avec Windows" : "Start with Windows"; } }
        public static string Quit      { get { return Fr ? "Quitter" : "Quit"; } }
        public static string Scale     { get { return Fr ? "Balance : " : "Scale: "; } }
        public static string NoScale   { get { return Fr ? "Balance : pas encore choisie" : "Scale: not chosen yet"; } }
        public static string PortBusy  { get { return Fr ? "Le port 47800 est déjà utilisé : le connecteur ne peut pas démarrer."
                                                         : "Port 47800 is already in use: the connector cannot start."; } }
        public static string Setup     { get { return Fr ? "Balance injoignable" : "Scale unreachable"; } }
        public static string SetupHelp { get { return Fr ? "Indiquez l’adresse de la balance (affichée dans Réglages → Réseau), ou lancez une recherche sur le réseau. La balance doit être allumée, sur le même réseau, avec un firmware qui a l’outil Pèse-colis connecté."
                                                         : "Enter the scale's address (shown in Settings → Network), or search the network. The scale must be on, on the same network, with a firmware that has the connected parcel scale."; } }
        public static string Save      { get { return Fr ? "Enregistrer" : "Save"; } }
        public static string Search    { get { return Fr ? "Rechercher" : "Search"; } }
        public static string Searching { get { return Fr ? "Recherche en cours…" : "Searching…"; } }
        public static string NotFound  { get { return Fr ? "Aucune balance trouvée." : "No scale found."; } }
    }

    // ---------------------------------------------------------------------
    // Settings: one line, scale=<address>, in %APPDATA%\TigerScaleConnector.
    // ---------------------------------------------------------------------
    static class Settings
    {
        static readonly object Gate = new object();
        static string scale;
        static string FilePath
        {
            get
            {
                string dir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                                          "TigerScaleConnector");
                Directory.CreateDirectory(dir);
                return Path.Combine(dir, "config.txt");
            }
        }

        public static string Scale
        {
            get
            {
                lock (Gate)
                {
                    if (scale == null)
                    {
                        scale = "";
                        try
                        {
                            foreach (string line in File.ReadAllLines(FilePath))
                                if (line.StartsWith("scale=")) scale = line.Substring(6).Trim();
                        }
                        catch (Exception) { }
                    }
                    return scale;
                }
            }
            set
            {
                lock (Gate)
                {
                    scale = Clean(value);
                    try { File.WriteAllText(FilePath, "scale=" + scale + "\r\n"); } catch (Exception) { }
                }
            }
        }

        // A host name or an IPv4 address, optionally with a port -- never a URL
        // with a path, so the relay can only ever reach http://<that>/api/...
        public static string Clean(string s)
        {
            s = (s ?? "").Trim();
            if (s.StartsWith("http://")) s = s.Substring(7);
            int slash = s.IndexOf('/');
            if (slash >= 0) s = s.Substring(0, slash);
            return Regex.IsMatch(s, @"^[A-Za-z0-9.\-]{1,253}(:[0-9]{1,5})?$") ? s : "";
        }

        const string RunKey = @"Software\Microsoft\Windows\CurrentVersion\Run";
        public static bool StartWithWindows
        {
            get
            {
                using (RegistryKey k = Registry.CurrentUser.OpenSubKey(RunKey))
                    return k != null && k.GetValue("TigerScaleConnector") != null;
            }
            set
            {
                using (RegistryKey k = Registry.CurrentUser.CreateSubKey(RunKey))
                {
                    if (value) k.SetValue("TigerScaleConnector", "\"" + Application.ExecutablePath + "\"");
                    else k.DeleteValue("TigerScaleConnector", false);
                }
            }
        }
    }

    static class Browser
    {
        public static void Open(string url)
        {
            try { Process.Start(new ProcessStartInfo(url) { UseShellExecute = true }); } catch (Exception) { }
        }
    }

    // ---------------------------------------------------------------------
    // The tray icon, and the UI thread every paste runs on.
    // ---------------------------------------------------------------------
    class Tray : ApplicationContext
    {
        readonly NotifyIcon icon;
        readonly ToolStripMenuItem scaleItem;
        readonly Control ui;            // a handle on the UI thread, for Invoke
        readonly HttpServer server;

        public Tray()
        {
            ui = new Control();
            ui.CreateControl();
            IntPtr force = ui.Handle;   // the handle must exist before Invoke
            try { Focus.Start(); } catch (Exception) { }   // pasting into "previous" then finds no window, nothing worse

            ContextMenuStrip menu = new ContextMenuStrip();
            ToolStripMenuItem open = new ToolStripMenuItem(T.Open);
            open.Font = new Font(open.Font, FontStyle.Bold);
            open.Click += delegate { OpenPage(); };
            scaleItem = new ToolStripMenuItem("") { Enabled = false };
            ToolStripMenuItem startup = new ToolStripMenuItem(T.Startup);
            startup.Checked = Settings.StartWithWindows;
            startup.Click += delegate
            {
                try { Settings.StartWithWindows = !startup.Checked; } catch (Exception) { }
                startup.Checked = Settings.StartWithWindows;
            };
            ToolStripMenuItem quit = new ToolStripMenuItem(T.Quit);
            quit.Click += delegate { ExitThread(); };
            menu.Items.Add(open);
            menu.Items.Add(scaleItem);
            menu.Items.Add(new ToolStripSeparator());
            menu.Items.Add(startup);
            menu.Items.Add(quit);
            menu.Opening += delegate { RefreshScaleItem(); };

            icon = new NotifyIcon();
            icon.Icon = MakeIcon();
            icon.Text = "TigerScale Connector";
            icon.ContextMenuStrip = menu;
            icon.DoubleClick += delegate { OpenPage(); };
            icon.Visible = true;
            RefreshScaleItem();

            server = new HttpServer(this);
            if (!server.Start())
            {
                MessageBox.Show(T.PortBusy, "TigerScale Connector", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                icon.Visible = false;
                Environment.Exit(1);
            }
            // Started by hand (not by Windows at logon): show the page at once.
            if (!Environment.CommandLine.Contains("--background")) OpenPage();
        }

        void RefreshScaleItem()
        {
            string s = Settings.Scale;
            scaleItem.Text = s.Length > 0 ? T.Scale + s : T.NoScale;
        }

        void OpenPage() { Browser.Open("http://127.0.0.1:" + Program.Port + "/"); }

        public T2 OnUi<T2>(Func<T2> f) { return (T2)ui.Invoke(f); }

        protected override void ExitThreadCore()
        {
            server.Stop();
            icon.Visible = false;
            icon.Dispose();
            base.ExitThreadCore();
        }

        // An orange disc with a white "T": no image file to ship.
        static Icon MakeIcon()
        {
            using (Bitmap b = new Bitmap(32, 32))
            {
                using (Graphics g = Graphics.FromImage(b))
                {
                    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
                    g.Clear(Color.Transparent);
                    using (SolidBrush o = new SolidBrush(Color.FromArgb(0xE8, 0x82, 0x1E)))
                        g.FillEllipse(o, 1, 1, 30, 30);
                    using (Font f = new Font("Segoe UI", 15, FontStyle.Bold, GraphicsUnit.Pixel))
                    using (StringFormat sf = new StringFormat { Alignment = StringAlignment.Center, LineAlignment = StringAlignment.Center })
                        g.DrawString("T", f, Brushes.White, new RectangleF(0, 0, 32, 32), sf);
                }
                return Icon.FromHandle(b.GetHicon());
            }
        }
    }

    // ---------------------------------------------------------------------
    // Which window was in front before the browser, and getting a window to
    // the front from a background program.
    // ---------------------------------------------------------------------
    static class Focus
    {
        delegate void WinEventProc(IntPtr hook, uint ev, IntPtr hwnd, int obj, int child, uint thread, uint time);
        [DllImport("user32.dll")] static extern IntPtr SetWinEventHook(uint min, uint max, IntPtr mod, WinEventProc proc, uint pid, uint tid, uint flags);
        [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
        [DllImport("user32.dll")] static extern bool SetForegroundWindow(IntPtr h);
        [DllImport("user32.dll")] static extern bool BringWindowToTop(IntPtr h);
        [DllImport("user32.dll")] static extern bool ShowWindow(IntPtr h, int cmd);
        [DllImport("user32.dll")] static extern bool IsIconic(IntPtr h);
        [DllImport("user32.dll")] static extern bool IsWindow(IntPtr h);
        [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr h);
        [DllImport("user32.dll")] static extern IntPtr GetAncestor(IntPtr h, uint flags);
        [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
        [DllImport("user32.dll")] static extern bool AttachThreadInput(uint a, uint b, bool attach);
        [DllImport("kernel32.dll")] static extern uint GetCurrentThreadId();
        [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetClassName(IntPtr h, StringBuilder s, int n);
        [DllImport("user32.dll", CharSet = CharSet.Unicode)] static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
        [DllImport("user32.dll")] static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);

        const uint EVENT_SYSTEM_FOREGROUND = 3, WINEVENT_OUTOFCONTEXT = 0, GA_ROOT = 2;
        static WinEventProc keep;       // the delegate must outlive the hook
        static IntPtr cur, prev;
        static readonly uint ownPid = (uint)Process.GetCurrentProcess().Id;

        public static void Start()
        {
            keep = OnForeground;
            SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, IntPtr.Zero, keep, 0, 0, WINEVENT_OUTOFCONTEXT);
            cur = GetForegroundWindow();
        }

        static void OnForeground(IntPtr hook, uint ev, IntPtr hwnd, int obj, int child, uint thread, uint time)
        {
            if (hwnd == IntPtr.Zero) return;
            IntPtr root = GetAncestor(hwnd, GA_ROOT);
            if (root == IntPtr.Zero || root == cur || !Usable(root)) return;
            prev = cur;
            cur = root;
        }

        // The taskbar, the tray overflow and this program's own menu are never
        // where the user meant the weight to go.
        static bool Usable(IntPtr h)
        {
            if (!IsWindowVisible(h)) return false;
            uint pid; GetWindowThreadProcessId(h, out pid);
            if (pid == ownPid) return false;
            string c = ClassOf(h);
            return c != "Shell_TrayWnd" && c != "Shell_SecondaryTrayWnd" && c != "NotifyIconOverflowWindow"
                && c != "TopLevelWindowForOverflowXamlIsland" && c != "Progman" && c != "WorkerW";
        }

        static string ClassOf(IntPtr h) { StringBuilder s = new StringBuilder(256); GetClassName(h, s, 256); return s.ToString(); }
        public static string TitleOf(IntPtr h) { StringBuilder s = new StringBuilder(256); GetWindowText(h, s, 256); return s.ToString(); }

        // The page asks for "previous" when it has the focus itself (a click
        // on Paste): the user means the window they were in just before.
        public static IntPtr Previous()
        {
            IntPtr fg = GetAncestor(GetForegroundWindow(), GA_ROOT);
            IntPtr h = fg == cur ? prev : cur;
            return h != IntPtr.Zero && IsWindow(h) ? h : IntPtr.Zero;
        }

        public static IntPtr Current()
        {
            IntPtr h = GetAncestor(GetForegroundWindow(), GA_ROOT);
            return h != IntPtr.Zero && Usable(h) ? h : IntPtr.Zero;
        }

        // Windows lets a background program take the foreground only in narrow
        // cases. Sharing the input state of the window in front is the usual
        // way in; a tap of Alt is the fallback when that is not enough.
        public static bool Bring(IntPtr h)
        {
            if (IsIconic(h)) ShowWindow(h, 9 /* SW_RESTORE */);
            if (GetForegroundWindow() == h) return true;
            uint dummy;
            uint fgThread = GetWindowThreadProcessId(GetForegroundWindow(), out dummy);
            uint me = GetCurrentThreadId();
            bool attached = fgThread != 0 && fgThread != me && AttachThreadInput(me, fgThread, true);
            BringWindowToTop(h);
            SetForegroundWindow(h);
            if (attached) AttachThreadInput(me, fgThread, false);
            if (!WaitFront(h, 300))
            {
                keybd_event(0x12, 0, 0, UIntPtr.Zero);       // Alt down
                keybd_event(0x12, 0, 2, UIntPtr.Zero);       // Alt up
                SetForegroundWindow(h);
            }
            return WaitFront(h, 600);
        }

        static bool WaitFront(IntPtr h, int ms)
        {
            for (int t = 0; t < ms; t += 20)
            {
                if (GetAncestor(GetForegroundWindow(), GA_ROOT) == h) return true;
                Thread.Sleep(20);
            }
            return false;
        }
    }

    // ---------------------------------------------------------------------
    // Typing: Unicode key events, so the keyboard layout (AZERTY, QWERTY…)
    // does not matter and the clipboard is left alone.
    // ---------------------------------------------------------------------
    static class Keys2
    {
        [StructLayout(LayoutKind.Sequential)] struct MOUSEINPUT { public int dx, dy; public uint data, flags, time; public IntPtr extra; }
        [StructLayout(LayoutKind.Sequential)] struct KEYBDINPUT { public ushort vk, scan; public uint flags, time; public IntPtr extra; }
        [StructLayout(LayoutKind.Explicit)] struct UNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
        [StructLayout(LayoutKind.Sequential)] struct INPUT { public uint type; public UNION u; }
        [DllImport("user32.dll", SetLastError = true)] static extern uint SendInput(uint n, INPUT[] inputs, int size);
        [DllImport("user32.dll")] static extern short GetAsyncKeyState(int vk);

        const uint KEYEVENTF_KEYUP = 2, KEYEVENTF_UNICODE = 4;

        static INPUT Key(ushort vk, ushort scan, uint flags)
        {
            INPUT i = new INPUT();
            i.type = 1; // INPUT_KEYBOARD
            i.u.ki.vk = vk; i.u.ki.scan = scan; i.u.ki.flags = flags;
            return i;
        }

        // A held Ctrl or Alt would turn "1" into a shortcut.
        public static void WaitModifiersUp()
        {
            int[] mods = { 0x10, 0x11, 0x12, 0x5B, 0x5C };
            for (int t = 0; t < 1500; t += 20)
            {
                bool any = false;
                foreach (int m in mods) if ((GetAsyncKeyState(m) & 0x8000) != 0) any = true;
                if (!any) return;
                Thread.Sleep(20);
            }
        }

        public static bool Type(string text, string after)
        {
            List<INPUT> list = new List<INPUT>();
            foreach (char c in text)
            {
                list.Add(Key(0, c, KEYEVENTF_UNICODE));
                list.Add(Key(0, c, KEYEVENTF_UNICODE | KEYEVENTF_KEYUP));
            }
            ushort vk = after == "tab" ? (ushort)0x09 : after == "enter" ? (ushort)0x0D : (ushort)0;
            if (vk != 0) { list.Add(Key(vk, 0, 0)); list.Add(Key(vk, 0, KEYEVENTF_KEYUP)); }
            INPUT[] a = list.ToArray();
            return SendInput((uint)a.Length, a, Marshal.SizeOf(typeof(INPUT))) == a.Length;
        }
    }

    // ---------------------------------------------------------------------
    // Finding the "chosen program": a running process, by full path or by
    // name. It is never started -- a request that could launch an arbitrary
    // path is not something a page should be able to make.
    // ---------------------------------------------------------------------
    static class Apps
    {
        [DllImport("kernel32.dll")] static extern IntPtr OpenProcess(uint access, bool inherit, uint pid);
        [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr h);
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode)] static extern bool QueryFullProcessImageName(IntPtr h, uint flags, StringBuilder s, ref uint n);
        delegate bool EnumProc(IntPtr h, IntPtr l);
        [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc p, IntPtr l);
        [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr h);
        [DllImport("user32.dll")] static extern IntPtr GetWindow(IntPtr h, uint cmd);

        static string PathOf(int pid)
        {
            IntPtr h = OpenProcess(0x1000 /* QUERY_LIMITED_INFORMATION */, false, (uint)pid);
            if (h == IntPtr.Zero) return "";
            try
            {
                StringBuilder s = new StringBuilder(1024); uint n = 1024;
                return QueryFullProcessImageName(h, 0, s, ref n) ? s.ToString() : "";
            }
            finally { CloseHandle(h); }
        }

        public static IntPtr Find(string want)
        {
            want = want.Trim().Trim('"');
            bool byPath = want.IndexOf('\\') >= 0 || want.IndexOf('/') >= 0;
            string name = Path.GetFileNameWithoutExtension(want);
            string full = "";
            if (byPath) { try { full = Path.GetFullPath(want); } catch (Exception) { return IntPtr.Zero; } }
            foreach (Process p in Process.GetProcesses())
            {
                try
                {
                    if (!string.Equals(p.ProcessName, name, StringComparison.OrdinalIgnoreCase)) continue;
                    if (byPath && !string.Equals(PathOf(p.Id), full, StringComparison.OrdinalIgnoreCase)) continue;
                    IntPtr w = p.MainWindowHandle;
                    if (w == IntPtr.Zero) w = AnyWindowOf(p.Id);
                    if (w != IntPtr.Zero) return w;
                }
                catch (Exception) { }
                finally { p.Dispose(); }
            }
            return IntPtr.Zero;
        }

        static IntPtr AnyWindowOf(int pid)
        {
            IntPtr found = IntPtr.Zero;
            EnumWindows(delegate (IntPtr h, IntPtr l)
            {
                uint p; Focus.GetWindowThreadProcessId(h, out p);
                if (p == pid && IsWindowVisible(h) && GetWindow(h, 4 /* GW_OWNER */) == IntPtr.Zero) { found = h; return false; }
                return true;
            }, IntPtr.Zero);
            return found;
        }
    }

    // ---------------------------------------------------------------------
    // The HTTP side: a minimal server on 127.0.0.1 (no HttpListener, which
    // needs a URL reservation on some machines), and the relay to the scale.
    // ---------------------------------------------------------------------
    class HttpServer
    {
        readonly Tray tray;
        TcpListener listener;
        volatile bool running;
        static readonly Regex Weight = new Regex(@"^[0-9]{1,6}([.,][0-9]{1,3})?( ?(g|kg))?$");

        public HttpServer(Tray t) { tray = t; }

        public bool Start()
        {
            try
            {
                listener = new TcpListener(IPAddress.Loopback, Program.Port);
                listener.Start();
            }
            catch (SocketException) { return false; }
            running = true;
            Thread th = new Thread(Loop) { IsBackground = true, Name = "http" };
            th.Start();
            return true;
        }

        public void Stop() { running = false; try { listener.Stop(); } catch (Exception) { } }

        void Loop()
        {
            while (running)
            {
                TcpClient c;
                try { c = listener.AcceptTcpClient(); } catch (Exception) { return; }
                ThreadPool.QueueUserWorkItem(delegate { Serve(c); });
            }
        }

        class Req { public string Method, Path, Host, Origin; public byte[] Body = new byte[0]; }

        static Req Read(Stream s)
        {
            // Headers up to 8 KB, body up to 4 KB: nothing this page sends is larger.
            MemoryStream head = new MemoryStream();
            int b;
            uint last4 = 0;                         // the last four bytes read
            bool ended = false;
            while (head.Length < 8192 && (b = s.ReadByte()) >= 0)
            {
                head.WriteByte((byte)b);
                last4 = (last4 << 8) | (uint)b;
                if (last4 == 0x0D0A0D0A) { ended = true; break; }   // \r\n\r\n
            }
            if (!ended) return null;
            string[] lines = Encoding.ASCII.GetString(head.ToArray()).Split(new[] { "\r\n" }, StringSplitOptions.None);
            string[] first = lines[0].Split(' ');
            if (first.Length < 2) return null;
            Req r = new Req { Method = first[0], Path = first[1] };
            int len = 0;
            for (int i = 1; i < lines.Length; i++)
            {
                int k = lines[i].IndexOf(':');
                if (k <= 0) continue;
                string n = lines[i].Substring(0, k).Trim().ToLowerInvariant(), v = lines[i].Substring(k + 1).Trim();
                if (n == "host") r.Host = v;
                else if (n == "origin") r.Origin = v;
                else if (n == "content-length") int.TryParse(v, out len);
            }
            if (len < 0 || len > 4096) return null;
            r.Body = new byte[len];
            int got = 0;
            while (got < len) { int n = s.Read(r.Body, got, len - got); if (n <= 0) return null; got += n; }
            return r;
        }

        static void Send(Stream s, int code, string type, byte[] body)
        {
            string status = code == 200 ? "OK" : code == 403 ? "Forbidden" : code == 404 ? "Not Found" : code == 502 ? "Bad Gateway" : "Bad Request";
            string h = "HTTP/1.1 " + code + " " + status + "\r\nContent-Type: " + type +
                       "\r\nContent-Length: " + body.Length + "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
            byte[] hb = Encoding.ASCII.GetBytes(h);
            s.Write(hb, 0, hb.Length);
            s.Write(body, 0, body.Length);
        }

        static void SendJson(Stream s, int code, string json) { Send(s, code, "application/json", Encoding.UTF8.GetBytes(json)); }
        static void SendHtml(Stream s, string html) { Send(s, 200, "text/html; charset=utf-8", Encoding.UTF8.GetBytes(html)); }

        static bool OwnHost(string h) { return h == "127.0.0.1:" + Program.Port || h == "localhost:" + Program.Port; }
        static bool OwnOrigin(string o) { return o == "http://127.0.0.1:" + Program.Port || o == "http://localhost:" + Program.Port; }

        void Serve(TcpClient c)
        {
            using (c)
            {
                try
                {
                    c.ReceiveTimeout = 5000; c.SendTimeout = 5000;
                    NetworkStream s = c.GetStream();
                    Req r = Read(s);
                    if (r == null) return;
                    if (!OwnHost(r.Host)) { SendJson(s, 403, "{\"error\":\"host\"}"); return; }
                    if (r.Method == "POST" && !OwnOrigin(r.Origin)) { SendJson(s, 403, "{\"error\":\"origin\"}"); return; }
                    string path = r.Path.Split('?')[0];

                    if (r.Method == "GET" && path == "/hello")
                        SendJson(s, 200, "{\"app\":\"tigerscale-connector\",\"version\":\"" + Program.Version + "\"}");
                    else if (r.Method == "GET" && (path == "/" || path == "/colis"))
                    {
                        byte[] page = Relay("GET", "/colis", 2500);
                        if (page != null) Send(s, 200, "text/html; charset=utf-8", page);
                        else SendHtml(s, SetupPage());
                    }
                    else if (r.Method == "GET" && path == "/api/parcel")
                    {
                        byte[] j = Relay("GET", "/api/parcel", 1500);
                        if (j != null) Send(s, 200, "application/json", j); else SendJson(s, 502, "{\"error\":\"scale\"}");
                    }
                    else if (r.Method == "POST" && path == "/api/tare")
                    {
                        byte[] j = Relay("POST", "/api/tare", 3000);
                        if (j != null) Send(s, 200, "application/json", j); else SendJson(s, 502, "{\"error\":\"scale\"}");
                    }
                    else if (r.Method == "GET" && path == "/config")
                        SendJson(s, 200, "{\"scale\":" + Json(Settings.Scale) + "}");
                    else if (r.Method == "POST" && path == "/config")
                        SendJson(s, 200, Config(Parse(r.Body)));
                    else if (r.Method == "POST" && path == "/paste")
                        SendJson(s, 200, Paste(Parse(r.Body)));
                    else
                        SendJson(s, 404, "{\"error\":\"not found\"}");
                }
                catch (Exception) { }
            }
        }

        static Dictionary<string, object> Parse(byte[] body)
        {
            try { return new JavaScriptSerializer().Deserialize<Dictionary<string, object>>(Encoding.UTF8.GetString(body)) ?? new Dictionary<string, object>(); }
            catch (Exception) { return new Dictionary<string, object>(); }
        }

        static string Str(Dictionary<string, object> d, string k)
        {
            object v; return d.TryGetValue(k, out v) && v is string ? (string)v : "";
        }

        static string Json(string s)
        {
            StringBuilder b = new StringBuilder("\"");
            foreach (char c in s ?? "")
            {
                if (c == '"' || c == '\\') b.Append('\\').Append(c);
                else if (c < 0x20 || c == '<' || c == '>') b.Append("\\u").Append(((int)c).ToString("x4"));
                else b.Append(c);
            }
            return b.Append('"').ToString();
        }

        // ---- the relay ---------------------------------------------------
        static byte[] Relay(string method, string path, int timeoutMs)
        {
            string scale = Settings.Scale;
            return scale.Length == 0 ? null : Fetch(method, "http://" + scale + path, timeoutMs);
        }

        static byte[] Fetch(string method, string url, int timeoutMs)
        {
            try
            {
                HttpWebRequest q = (HttpWebRequest)WebRequest.Create(url);
                q.Method = method;
                q.Proxy = null;                 // the scale is on the LAN, never behind a proxy
                q.Timeout = timeoutMs; q.ReadWriteTimeout = timeoutMs;
                q.KeepAlive = false;
                if (method == "POST") q.ContentLength = 0;
                using (HttpWebResponse a = (HttpWebResponse)q.GetResponse())
                using (Stream st = a.GetResponseStream())
                using (MemoryStream m = new MemoryStream())
                {
                    if ((int)a.StatusCode != 200) return null;
                    st.CopyTo(m);
                    return m.ToArray();
                }
            }
            catch (Exception) { return null; }
        }

        // ---- settings, and finding the scale on the network --------------
        string Config(Dictionary<string, object> d)
        {
            object scan;
            if (d.TryGetValue("scan", out scan) && scan is bool && (bool)scan)
            {
                string found = Scan();
                if (found.Length > 0) Settings.Scale = found;
                return "{\"scale\":" + Json(found) + "}";
            }
            Settings.Scale = Str(d, "scale");
            return "{\"scale\":" + Json(Settings.Scale) + "}";
        }

        // Every /24 this PC sits on, 64 at a time: an address answers if its
        // /api/parcel returns a parcel frame, which only this firmware serves.
        static string Scan()
        {
            List<string> hosts = new List<string>();
            foreach (NetworkInterface ni in NetworkInterface.GetAllNetworkInterfaces())
            {
                if (ni.OperationalStatus != OperationalStatus.Up || ni.NetworkInterfaceType == NetworkInterfaceType.Loopback) continue;
                foreach (UnicastIPAddressInformation a in ni.GetIPProperties().UnicastAddresses)
                {
                    if (a.Address.AddressFamily != AddressFamily.InterNetwork) continue;
                    byte[] b = a.Address.GetAddressBytes();
                    bool priv = b[0] == 10 || (b[0] == 172 && b[1] >= 16 && b[1] <= 31) || (b[0] == 192 && b[1] == 168);
                    if (!priv) continue;
                    for (int i = 1; i < 255; i++)
                    {
                        string h = b[0] + "." + b[1] + "." + b[2] + "." + i;
                        if (i != b[3] && !hosts.Contains(h)) hosts.Add(h);
                    }
                }
            }
            string hit = "";
            Parallel.ForEach(hosts, new ParallelOptions { MaxDegreeOfParallelism = 64 }, delegate (string h, ParallelLoopState st)
            {
                byte[] j = Fetch("GET", "http://" + h + "/api/parcel", 900);
                if (j != null && Encoding.UTF8.GetString(j).Contains("\"grams\""))
                {
                    lock (hosts) { if (hit.Length == 0) hit = h; }
                    st.Stop();
                }
            });
            return hit;
        }

        // ---- the paste ---------------------------------------------------
        string Paste(Dictionary<string, object> d)
        {
            string text = Str(d, "text"), target = Str(d, "target"), app = Str(d, "app"), after = Str(d, "after");
            if (!Weight.IsMatch(text)) return "{\"ok\":false,\"error\":\"text\"}";
            if (after != "tab" && after != "enter") after = "none";
            return tray.OnUi<string>(delegate
            {
                IntPtr w = target == "app" ? Apps.Find(app)
                         : target == "current" ? Focus.Current()
                         : Focus.Previous();
                if (w == IntPtr.Zero)
                    return "{\"ok\":false,\"error\":" + Json(target == "app" ? "app_not_running" : "no_window") + "}";
                if (!Focus.Bring(w)) return "{\"ok\":false,\"error\":\"focus\"}";
                Keys2.WaitModifiersUp();
                bool ok = Keys2.Type(text, after);
                return ok ? "{\"ok\":true,\"window\":" + Json(Focus.TitleOf(w)) + "}"
                          : "{\"ok\":false,\"error\":\"input\"}";
            });
        }

        // Shown instead of the scale's page while the scale cannot be reached.
        static string SetupPage()
        {
            return "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">" +
                "<title>TigerScale</title><style>:root{--bg:#0f1115;--card:#1a1d24;--border:#2c313b;--text:#e8eaef;--muted:#9aa1ad;--accent:#e8821e}" +
                "@media (prefers-color-scheme:light){:root{--bg:#f4f5f7;--card:#fff;--border:#d9dce2;--text:#1b1e24;--muted:#5f6672}}" +
                "*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.4 system-ui,sans-serif}" +
                "main{max-width:560px;margin:0 auto;padding:20px 16px}p{color:var(--muted)}" +
                "input,button{font:inherit;color:var(--text);background:var(--bg);border:1px solid var(--border);border-radius:10px;min-height:44px;padding:0 12px;width:100%;margin-top:8px}" +
                "button{cursor:pointer;font-weight:600}.pri{background:var(--accent);border-color:var(--accent);color:#fff}</style></head><body><main>" +
                "<h1>" + Html(T.Setup) + "</h1><p>" + Html(T.SetupHelp) + "</p>" +
                "<input id=\"s\" placeholder=\"tigerscale-XXXX.local\" spellcheck=\"false\" value=\"" + Html(Settings.Scale) + "\">" +
                "<button class=\"pri\" id=\"save\">" + Html(T.Save) + "</button><button id=\"scan\">" + Html(T.Search) + "</button><p id=\"m\"></p>" +
                "<script>const m=document.getElementById('m');" +
                "function post(b){return fetch('/config',{method:'POST',headers:{'Content-Type':'text/plain'},body:JSON.stringify(b)}).then(r=>r.json())}" +
                "document.getElementById('save').onclick=()=>post({scale:document.getElementById('s').value.trim()}).then(()=>location.reload());" +
                "document.getElementById('scan').onclick=()=>{m.textContent=" + Json(T.Searching) + ";post({scan:true}).then(j=>{if(j.scale)location.reload();else m.textContent=" + Json(T.NotFound) + "})};" +
                "</script></main></body></html>";
        }

        static string Html(string s) { return WebUtility.HtmlEncode(s ?? ""); }
    }
}
