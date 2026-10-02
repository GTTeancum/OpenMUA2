using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Security.Cryptography;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Runtime.InteropServices;

internal static class OpenMUA2
{
    static string local = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "OpenMUA2");
    static string Hash(Stream stream) { using (var sha = SHA256.Create()) return BitConverter.ToString(sha.ComputeHash(stream)).Replace("-", "").ToLowerInvariant(); }
    static string FileHash(string path) { using (var f = File.OpenRead(path)) return Hash(f); }
    static string Quote(string path) { return "\"" + path + "\""; }
    static string Prepare()
    {
        var assembly = Assembly.GetExecutingAssembly();
        string id;
        using (var s = assembly.GetManifestResourceStream("payload.zip")) id = Hash(s);
        string cache = Path.Combine(local, "runtime", id);
        using (var mutex = new Mutex(false, "Local\\OpenMUA2-Package-" + id))
        {
            try { mutex.WaitOne(); } catch (AbandonedMutexException) { }
            try
            {
                var hashes = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
                using (var reader = new StreamReader(assembly.GetManifestResourceStream("payload.manifest")))
                {
                    string line;
                    while ((line = reader.ReadLine()) != null) { var fields = line.Split('\t'); hashes.Add(fields[1], fields[0]); }
                }
                using (var s = assembly.GetManifestResourceStream("payload.zip"))
                using (var zip = new ZipArchive(s, ZipArchiveMode.Read))
                {
                    if (zip.Entries.Count != hashes.Count) throw new InvalidDataException("Package manifest count mismatch.");
                    foreach (var entry in zip.Entries)
                    {
                        string target = Path.GetFullPath(Path.Combine(cache, entry.FullName.Replace('/', Path.DirectorySeparatorChar)));
                        if (!target.StartsWith(cache + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase)) throw new InvalidDataException("Invalid package path.");
                        string expected;
                        if (!hashes.TryGetValue(entry.FullName, out expected)) throw new InvalidDataException("Missing package hash.");
                        if (File.Exists(target) && FileHash(target) == expected) continue;
                        Directory.CreateDirectory(Path.GetDirectoryName(target));
                        string temp = target + ".pending";
                        using (var input = entry.Open()) using (var output = File.Create(temp)) input.CopyTo(output);
                        if (FileHash(temp) != expected) throw new InvalidDataException("Package checksum failed: " + entry.FullName);
                        File.Copy(temp, target, true);
                        File.Delete(temp);
                    }
                }
                return cache;
            }
            finally { mutex.ReleaseMutex(); }
        }
    }

    static ProcessStartInfo StartInfo(string cache, string arguments)
    {
        var info = new ProcessStartInfo(Path.Combine(cache, "moderngekko-run.exe"), arguments);
        info.WorkingDirectory = cache;
        info.UseShellExecute = false;
        info.CreateNoWindow = true;
        info.RedirectStandardOutput = true;
        info.RedirectStandardError = true;
        var remove = new List<string>();
        foreach (string key in info.EnvironmentVariables.Keys)
            if (key.StartsWith("OPENMUA2_", StringComparison.OrdinalIgnoreCase) || key.StartsWith("MODERNGEKKO_", StringComparison.OrdinalIgnoreCase)) remove.Add(key);
        foreach (string key in remove) info.EnvironmentVariables.Remove(key);
        info.EnvironmentVariables["MODERNGEKKO_SIMPLE_FORMAT"] = "on";
        info.EnvironmentVariables["OPENMUA2_FORMAT_IDENTITY_CACHE"] = "0";
        info.EnvironmentVariables["OPENMUA2_JIT_BUDGET_US"] = "0";
        info.EnvironmentVariables["OPENMUA2_BACKPATCH_RESERVE"] = "0";
        return info;
    }

    static int Run(ProcessStartInfo info, string log)
    {
        using (var writer = new StreamWriter(log, false))
        using (var process = new Process())
        {
            process.StartInfo = info;
            object gate = new object();
            DataReceivedEventHandler receive = delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) lock (gate) { writer.WriteLine(e.Data); writer.Flush(); } };
            process.OutputDataReceived += receive;
            process.ErrorDataReceived += receive;
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
            return process.ExitCode;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    struct XPad { public ushort Buttons; public byte LT, RT; public short LX, LY, RX, RY; }
    [StructLayout(LayoutKind.Sequential)]
    struct XCaps { public byte Type, SubType; public ushort Flags; public XPad Pad; public ushort LeftMotor, RightMotor; }
    [DllImport("xinput1_4.dll")]
    static extern uint XInputGetCapabilities(uint index, uint flags, out XCaps caps);

    static string ConnectedXInput()
    {
        for (uint port = 0; port < 4; ++port) {
            XCaps caps;
            if (XInputGetCapabilities(port, 0, out caps) == 0 && caps.SubType == 1)
                return "XInput/" + port + "/Gamepad";
        }
        return null;
    }

    static Dictionary<string, string> ProfileSection(string text)
    {
        var values = new Dictionary<string, string>(StringComparer.Ordinal);
        bool active = false, seen = false;
        foreach (string raw in text.Replace("\r", "").Split('\n')) {
            string line = raw.Trim();
            if (line.Length == 0 || line.StartsWith("#") || line.StartsWith(";")) continue;
            if (line.StartsWith("[")) {
                active = line == "[Wiimote1]";
                if (active && seen) return null;
                seen |= active;
                continue;
            }
            if (!active) continue;
            int equals = line.IndexOf('=');
            if (equals < 1) return null;
            string key = line.Substring(0, equals).Trim();
            if (values.ContainsKey(key)) return null;
            values.Add(key, line.Substring(equals + 1).Trim());
        }
        return seen ? values : null;
    }

    static bool IsManagedProfile(string text, string body)
    {
        var actual = ProfileSection(text);
        var expected = ProfileSection("[Wiimote1]\n" + body);
        if (actual == null || expected == null || !actual.ContainsKey("Device")) return false;
        actual.Remove("Device");
        if (actual.Count != expected.Count) return false;
        foreach (var pair in expected) {
            string value;
            if (!actual.TryGetValue(pair.Key, out value) || value != pair.Value) return false;
        }
        return true;
    }

    static string ReplaceManagedDevice(string text, string device)
    {
        bool active = false;
        return System.Text.RegularExpressions.Regex.Replace(text, @"(?m)^[^\r\n]+", delegate(System.Text.RegularExpressions.Match match) {
            string line = match.Value.Trim();
            if (line.StartsWith("[")) active = line == "[Wiimote1]";
            int equals = line.IndexOf('=');
            if (active && equals > 0 && line.Substring(0, equals).Trim() == "Device")
                return "Device = " + device;
            return match.Value;
        });
    }

    static void EnsureControls(string cache, string saves, string diagnosticLog)
    {
        string profile = Path.Combine(saves, "Config", "WiimoteNew.ini");
        string current = File.Exists(profile) ? File.ReadAllText(profile) : "";
        string body;
        using (var reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Xbox-v5-profile.txt"))) body = reader.ReadToEnd();
        bool configured = !String.IsNullOrWhiteSpace(current);
        bool managed = configured && IsManagedProfile(current, body);
        if (configured && !managed) return;
        // Xbox controllers use the Windows XInput backend directly. SDL remains
        // a fallback for other gamepads; customized profiles are preserved.
        string device = ConnectedXInput();
        if (device == null && configured) return;
        if (device == null) {
            int result = Run(StartInfo(cache, "--controller-diagnostics --controller-diagnostics-seconds 0"), diagnosticLog);
            if (result != 0) throw new InvalidOperationException("Controller detection failed. Details: " + diagnosticLog);
            const string prefix = "controller diagnostics: selected=\"";
            foreach (string line in File.ReadAllLines(diagnosticLog))
                if (line.StartsWith(prefix, StringComparison.Ordinal) && line.EndsWith("\"", StringComparison.Ordinal))
                    device = line.Substring(prefix.Length, line.Length - prefix.Length - 1);
        }
        if (String.IsNullOrEmpty(device))
            throw new InvalidOperationException("Connect your Xbox controller, then reopen OpenMUA2. No gamepad was detected.");
        string next = managed ? ReplaceManagedDevice(current, device) :
            "[Wiimote1]\nDevice = " + device + "\n" + body + "[Wiimote2]\n[Wiimote3]\n[Wiimote4]\n[BalanceBoard]\n";
        if (next == current) return;
        Directory.CreateDirectory(Path.GetDirectoryName(profile));
        if (File.Exists(profile)) File.Copy(profile, profile + ".before-xinput-" + DateTime.UtcNow.Ticks + ".bak");
        File.WriteAllText(profile, next);
        File.WriteAllText(diagnosticLog, "Controller backend: " + device + "\r\n");
    }

    [STAThread]
    static int Main(string[] args)
    {
        bool verify = args.Length == 2 && args[0] == "--verify-package";
        try
        {
            if (args.Length != 0 && !verify) throw new ArgumentException("Unsupported launcher arguments.");
            var self = Process.GetCurrentProcess();
            if ((self.ProcessorAffinity.ToInt64() & 4) == 0) throw new InvalidOperationException("Logical CPU 2 is unavailable.");
            self.ProcessorAffinity = new IntPtr(4);
            string cache = Prepare();
            if (verify)
            {
                string report = Path.GetFullPath(args[1]);
                int result = Run(StartInfo(cache, "--help"), report + ".runner.log");
                if (result != 0) throw new InvalidOperationException("Packaged runner help failed: " + result);
                string help = File.ReadAllText(report + ".runner.log");
                if (!help.Contains("--game") || !help.Contains("--user-dir")) throw new InvalidDataException("Runner help was incomplete.");
                File.WriteAllText(report, "PASS: All embedded runtime files verified against SHA256 manifest.\r\nRunner --help exit: 0\r\nAffinity mask: " + self.ProcessorAffinity.ToInt64() + "\r\nRuntime cache: " + cache + "\r\nSaves path: " + Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "saves") + "\r\nGameplay was not launched. Visuals, audio and controller behavior were not tested.\r\n");
                return 0;
            }
            string root = AppDomain.CurrentDomain.BaseDirectory;
            string game = Path.Combine(root, "GameData");
            if (!File.Exists(Path.Combine(game, "sys", "main.dol"))) throw new FileNotFoundException("GameData is missing. Keep GameData next to OpenMUA2.exe.");
            string saves = Path.Combine(root, "saves");
            Directory.CreateDirectory(saves);
            string logs = Path.Combine(local, "Logs");
            Directory.CreateDirectory(logs);
            string session = DateTime.Now.ToString("yyyyMMdd-HHmmss-fff");
            string log = Path.Combine(logs, session + "-runtime.log");
            EnsureControls(cache, saves, Path.Combine(logs, session + "-controller.log"));
            var info = StartInfo(cache, "--game " + Quote(game) + " --cpu jit --title OpenMUA2 --user-dir " + Quote(saves) + " --graphics Vulkan --audio Cubeb");
            info.EnvironmentVariables["MODERNGEKKO_FRAME_TIMES"] = Path.Combine(logs, session + "-frames.csv");
            int exit = Run(info, log);
            if (exit != 0) MessageBox.Show("OpenMUA2 exited with code " + exit + ".\r\nDetails: " + log, "OpenMUA2", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return exit;
        }
        catch (Exception e)
        {
            if (verify) File.WriteAllText(Path.GetFullPath(args[1]), "FAIL: " + e.ToString());
            else MessageBox.Show("Unable to start OpenMUA2.\r\n" + e.Message, "OpenMUA2", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
    }
}
