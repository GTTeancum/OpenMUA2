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
    static string startupReport = "";
    static string Hash(Stream stream) { using (var sha = SHA256.Create()) return BitConverter.ToString(sha.ComputeHash(stream)).Replace("-", "").ToLowerInvariant(); }
    static string FileHash(string path) { using (var f = File.OpenRead(path)) return Hash(f); }
    static string Quote(string path) { return "\"" + path + "\""; }
    static int VerifyXboxUiAssets(string wad, TextReader manifest)
    {
        string header = manifest.ReadLine();
        int version = header == "OpenMUA2-Xbox-UI-v3" ? 3 :
                      header == "OpenMUA2-Xbox-UI-v4" ? 4 :
                      header == "OpenMUA2-Xbox-UI-v5" ? 5 :
                      header == "OpenMUA2-Xbox-UI-v6" ? 6 :
                      header == "OpenMUA2-Xbox-UI-v7" ? 7 :
                      header == "OpenMUA2-Xbox-UI-v8" ? 8 : 0;
        if (version == 0) throw new InvalidDataException("Unsupported Xbox UI asset version.");
        var required = new HashSet<string>(StringComparer.Ordinal) {
            "data/vv_tips.engb", "data/vv_tips.itab", "data/vv_tips.xmlb",
            "data/strings.engb", "data/strings.itab", "data/strings.xmlb",
            "packages/generated/maps/package/permanent.fb",
            "packages/generated/maps/package/permanent_rev.fb",
            "textures/fonts/rev_med_eng.igb", "textures/fonts/rev_med_ws_eng.igb"
        };
        if (version >= 4) {
            required.Add("ui/fonts/rev_med.xmlb");
            required.Add("ui/fonts/rev_med_ws.xmlb");
        }
        if (version >= 7) required.Add("packages/generated/maps/package/menus/options_rev.fb");
        if (version >= 8) required.Add("packages/generated/maps/package/menus/cw_pda_rev.fb");
        var expected = new Dictionary<string, string>(StringComparer.Ordinal);
        string line;
        while ((line = manifest.ReadLine()) != null) {
            var fields = line.Split('\t');
            if (fields.Length != 2 || fields[0].Length != 64 || !required.Remove(fields[1]))
                throw new InvalidDataException("Invalid Xbox UI manifest entry.");
            foreach (char c in fields[0]) if (!Uri.IsHexDigit(c))
                throw new InvalidDataException("Invalid Xbox UI checksum.");
            expected.Add(fields[1], fields[0].ToLowerInvariant());
        }
        if (required.Count != 0) throw new InvalidDataException("Incomplete Xbox UI manifest.");
        using (var stream = File.OpenRead(wad))
        using (var archive = new ZipArchive(stream, ZipArchiveMode.Read)) {
            var found = new HashSet<string>(StringComparer.Ordinal);
            foreach (var entry in archive.Entries) {
                string checksum;
                if (!expected.TryGetValue(entry.FullName, out checksum)) continue;
                if (!found.Add(entry.FullName)) throw new InvalidDataException("Duplicate Xbox UI archive entry.");
                using (var data = entry.Open()) if (Hash(data) != checksum)
                    throw new InvalidDataException("GameData Xbox UI does not match this executable: " + entry.FullName);
            }
            if (found.Count != expected.Count) throw new InvalidDataException("GameData Xbox UI assets are incomplete.");
        }
        return version;
    }

    static bool ConfigureXboxUi(ProcessStartInfo info, string game)
    {
        int version;
        using (var resource = Assembly.GetExecutingAssembly().GetManifestResourceStream("xbox-ui.manifest")) {
            if (resource == null) return false; // Older packages retain their original UI.
            using (var reader = new StreamReader(resource))
                version = VerifyXboxUiAssets(Path.Combine(game, "files", "z", "assetsfb.wad"), reader);
        }
        ConfigureXboxUiVersion(info, version);
        return true;
    }

    static void ConfigureXboxUiVersion(ProcessStartInfo info, int version)
    {
        info.EnvironmentVariables["OPENMUA2_XBOX_GLYPHS"] = "1";
        if (version >= 5) {
            // These switches require the exact paired HUD/font data verified above.
            info.EnvironmentVariables["OPENMUA2_GAMEPAD_PROVIDER"] = "1";
            info.EnvironmentVariables["OPENMUA2_DIRECT_GAMEPAD"] = "1";
            info.EnvironmentVariables["OPENMUA2_RAPID_TAP_GLYPHS"] = "1";
        }
        if (version >= 6) {
            // v6 pairs the shared A/B/X/Y HUD cells with complete local aiming.
            info.EnvironmentVariables["OPENMUA2_GAMEPAD_AIM"] = "1";
            info.EnvironmentVariables["OPENMUA2_GAMEPAD_LOCKON"] = "1";
        }
    }
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
            writer.WriteLine("OpenMUA2 session: " + DateTime.Now.ToString("O"));
            writer.WriteLine("Runtime SHA256: " + FileHash(info.FileName));
            if (startupReport.Length != 0) writer.WriteLine(startupReport);
            writer.Flush();
            process.StartInfo = info;
            object gate = new object();
            DataReceivedEventHandler receive = delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) lock (gate) { writer.WriteLine(e.Data); writer.Flush(); } };
            process.OutputDataReceived += receive;
            process.ErrorDataReceived += receive;
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
            writer.WriteLine("Runtime exit code: " + process.ExitCode);
            return process.ExitCode;
        }
    }

    // ---- First-run game setup -------------------------------------------------
    // The release carries only the OpenMUA2 archive members (content.zip) plus a
    // manifest describing the complete installed assetsfb.wad in order. The
    // player's own disc supplies every other member. Members are copied with
    // their compressed bytes untouched, then every member is hashed again.

    sealed class ContentEntry { public string Sha, Name; public uint Crc; public long Size; public bool Custom; }
    sealed class RawEntry { public string Name; public ushort Flags, Method, Time, Date; public uint Crc, Compressed, Size, Offset; }

    static List<ContentEntry> ContentManifest()
    {
        using (var resource = Assembly.GetExecutingAssembly().GetManifestResourceStream("content.manifest")) {
            if (resource == null) return null; // Development packages use a prepared GameData.
            var entries = new List<ContentEntry>();
            using (var reader = new StreamReader(resource)) {
                if (reader.ReadLine() != "OpenMUA2-Content-v1") throw new InvalidDataException("Unsupported content manifest.");
                string line;
                while ((line = reader.ReadLine()) != null) {
                    var f = line.Split('\t');
                    if (f.Length != 5 || f[0].Length != 64 || (f[3] != "d" && f[3] != "c")) throw new InvalidDataException("Invalid content manifest entry.");
                    entries.Add(new ContentEntry { Sha = f[0], Crc = Convert.ToUInt32(f[1], 16), Size = long.Parse(f[2]), Custom = f[3] == "c", Name = f[4] });
                }
            }
            return entries;
        }
    }

    static ushort U16(byte[] b, int o) { return (ushort)(b[o] | b[o + 1] << 8); }
    static uint U32(byte[] b, int o) { return (uint)(b[o] | b[o + 1] << 8 | b[o + 2] << 16 | b[o + 3] << 24); }

    static List<RawEntry> ReadCentralDirectory(Stream s)
    {
        int tail = (int)Math.Min(s.Length, 65557);
        var end = new byte[tail];
        s.Seek(s.Length - tail, SeekOrigin.Begin);
        ReadExactly(s, end, tail);
        int eocd = -1;
        for (int i = tail - 22; i >= 0; --i) if (U32(end, i) == 0x06054b50) { eocd = i; break; }
        if (eocd < 0) throw new InvalidDataException("Game archive is damaged (no directory).");
        int count = U16(end, eocd + 10);
        uint size = U32(end, eocd + 12), offset = U32(end, eocd + 16);
        var dir = new byte[size];
        s.Seek(offset, SeekOrigin.Begin);
        ReadExactly(s, dir, (int)size);
        var entries = new List<RawEntry>(count);
        for (int p = 0, i = 0; i < count; ++i) {
            if (U32(dir, p) != 0x02014b50) throw new InvalidDataException("Game archive directory is damaged.");
            int nameLength = U16(dir, p + 28), extra = U16(dir, p + 30), comment = U16(dir, p + 32);
            entries.Add(new RawEntry {
                Flags = U16(dir, p + 8), Method = U16(dir, p + 10), Time = U16(dir, p + 12), Date = U16(dir, p + 14),
                Crc = U32(dir, p + 16), Compressed = U32(dir, p + 20), Size = U32(dir, p + 24), Offset = U32(dir, p + 42),
                Name = System.Text.Encoding.UTF8.GetString(dir, p + 46, nameLength) });
            p += 46 + nameLength + extra + comment;
        }
        return entries;
    }

    static void ReadExactly(Stream s, byte[] buffer, int count)
    {
        for (int done = 0; done < count; ) {
            int n = s.Read(buffer, done, count - done);
            if (n <= 0) throw new EndOfStreamException("Unexpected end of game archive.");
            done += n;
        }
    }

    // Cheap identity check from the archive directory alone: names, order,
    // sizes and CRCs. Launch-time hashing of the Xbox UI members still follows.
    static bool ContentInstalled(string wad, List<ContentEntry> manifest)
    {
        if (!File.Exists(wad)) return false;
        List<RawEntry> entries;
        using (var s = File.OpenRead(wad)) entries = ReadCentralDirectory(s);
        if (entries.Count != manifest.Count) return false;
        for (int i = 0; i < entries.Count; ++i)
            if (entries[i].Name != manifest[i].Name || entries[i].Crc != manifest[i].Crc || entries[i].Size != manifest[i].Size) return false;
        return true;
    }

    static void CopyRawMember(Stream source, RawEntry entry, Stream output, BinaryWriter w, byte[] buffer)
    {
        var local = new byte[30];
        source.Seek(entry.Offset, SeekOrigin.Begin);
        ReadExactly(source, local, 30);
        if (U32(local, 0) != 0x04034b50) throw new InvalidDataException("Game archive member is damaged: " + entry.Name);
        source.Seek(entry.Offset + 30 + U16(local, 26) + U16(local, 28), SeekOrigin.Begin);
        var name = System.Text.Encoding.UTF8.GetBytes(entry.Name);
        entry.Offset = (uint)output.Position;
        entry.Flags &= 0x0800; // Sizes are known, so no trailing data descriptor.
        w.Write(0x04034b50u); w.Write((ushort)20); w.Write(entry.Flags); w.Write(entry.Method);
        w.Write(entry.Time); w.Write(entry.Date); w.Write(entry.Crc); w.Write(entry.Compressed); w.Write(entry.Size);
        w.Write((ushort)name.Length); w.Write((ushort)0); w.Write(name);
        w.Flush();
        for (long left = entry.Compressed; left > 0; ) {
            int n = source.Read(buffer, 0, (int)Math.Min(buffer.Length, left));
            if (n <= 0) throw new EndOfStreamException("Unexpected end of game archive.");
            output.Write(buffer, 0, n);
            left -= n;
        }
    }

    static void InstallContent(string game, List<ContentEntry> manifest, Action<string, double> report)
    {
        string wad = Path.Combine(game, "files", "z", "assetsfb.wad");
        string pending = wad + ".pending";
        report("Adding OpenMUA2 content", 0);
        using (var disc = File.OpenRead(wad))
        using (var content = Assembly.GetExecutingAssembly().GetManifestResourceStream("content.zip"))
        {
            var discEntries = new Dictionary<string, RawEntry>(StringComparer.Ordinal);
            foreach (var e in ReadCentralDirectory(disc)) discEntries[e.Name] = e;
            var customEntries = new Dictionary<string, RawEntry>(StringComparer.Ordinal);
            foreach (var e in ReadCentralDirectory(content)) customEntries[e.Name] = e;
            var written = new List<RawEntry>(manifest.Count);
            var buffer = new byte[1 << 20];
            using (var output = File.Create(pending))
            using (var w = new BinaryWriter(output))
            {
                for (int i = 0; i < manifest.Count; ++i) {
                    var m = manifest[i];
                    RawEntry e;
                    if (!(m.Custom ? customEntries : discEntries).TryGetValue(m.Name, out e) || e.Crc != m.Crc || e.Size != m.Size)
                        throw new InvalidDataException(m.Custom ? "OpenMUA2 content is incomplete: " + m.Name :
                            "This disc's game data doesn't match Marvel Ultimate Alliance 2 for Wii (USA): " + m.Name);
                    CopyRawMember(m.Custom ? content : disc, e, output, w, buffer);
                    written.Add(e);
                    report(null, 0.5 * (i + 1) / manifest.Count);
                }
                long directory = output.Position;
                foreach (var e in written) {
                    var name = System.Text.Encoding.UTF8.GetBytes(e.Name);
                    w.Write(0x02014b50u); w.Write((ushort)20); w.Write((ushort)20); w.Write(e.Flags); w.Write(e.Method);
                    w.Write(e.Time); w.Write(e.Date); w.Write(e.Crc); w.Write(e.Compressed); w.Write(e.Size);
                    w.Write((ushort)name.Length); w.Write((ushort)0); w.Write((ushort)0); w.Write((ushort)0); w.Write((ushort)0);
                    w.Write(0u); w.Write(e.Offset); w.Write(name);
                }
                long end = output.Position;
                w.Write(0x06054b50u); w.Write((ushort)0); w.Write((ushort)0);
                w.Write((ushort)written.Count); w.Write((ushort)written.Count);
                w.Write((uint)(end - directory)); w.Write((uint)directory); w.Write((ushort)0);
            }
        }
        report("Checking the finished game data", 0.5);
        using (var stream = File.OpenRead(pending))
        using (var archive = new ZipArchive(stream, ZipArchiveMode.Read))
        {
            if (archive.Entries.Count != manifest.Count) throw new InvalidDataException("Finished game archive is incomplete.");
            for (int i = 0; i < manifest.Count; ++i) {
                var entry = archive.Entries[i];
                if (entry.FullName != manifest[i].Name) throw new InvalidDataException("Finished game archive order is wrong: " + entry.FullName);
                using (var data = entry.Open()) if (Hash(data) != manifest[i].Sha)
                    throw new InvalidDataException(manifest[i].Custom ? "OpenMUA2 content failed its check: " + entry.FullName :
                        "This disc's game data doesn't match Marvel Ultimate Alliance 2 for Wii (USA): " + entry.FullName);
                report(null, 0.5 + 0.5 * (i + 1) / manifest.Count);
            }
        }
        File.Delete(wad);
        File.Move(pending, wad);
    }

    static string FindDiscImage(string root)
    {
        var images = new List<string>();
        foreach (string pattern in new[] { "*.wbfs", "*.iso", "*.rvz" })
            images.AddRange(Directory.GetFiles(root, pattern));
        if (images.Count == 1) return images[0];
        string chosen = null;
        using (var dialog = new OpenFileDialog()) {
            dialog.Title = images.Count == 0 ? "Select your Marvel Ultimate Alliance 2 (Wii, USA) disc image" :
                "Several disc images were found. Select Marvel Ultimate Alliance 2 (Wii, USA)";
            dialog.Filter = "Wii disc images (*.wbfs;*.iso;*.rvz)|*.wbfs;*.iso;*.rvz";
            dialog.InitialDirectory = root;
            if (dialog.ShowDialog() == DialogResult.OK) chosen = dialog.FileName;
        }
        if (chosen == null) throw new OperationCanceledException("Setup needs your Marvel Ultimate Alliance 2 (Wii, USA) disc image.");
        return chosen;
    }

    static void ExtractDisc(string cache, string image, string output, Action<string, double> report, Func<Process, bool> track)
    {
        var info = StartInfo(cache, "--extract-disc " + Quote(image) + " " + Quote(output));
        var errors = new System.Text.StringBuilder();
        using (var process = new Process())
        {
            process.StartInfo = info;
            process.OutputDataReceived += delegate(object sender, DataReceivedEventArgs e) {
                if (e.Data == null || !e.Data.StartsWith("progress ")) return;
                var f = e.Data.Split(' ');
                double done, total;
                if (f.Length == 3 && double.TryParse(f[1], out done) && double.TryParse(f[2], out total) && total > 0)
                    try { report(null, done / total); } catch (OperationCanceledException) { } // The process is being killed.
            };
            process.ErrorDataReceived += delegate(object sender, DataReceivedEventArgs e) { if (e.Data != null) lock (errors) errors.AppendLine(e.Data); };
            process.Start();
            if (!track(process)) { process.Kill(); throw new OperationCanceledException("Setup was canceled."); }
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            process.WaitForExit();
            track(null);
            if (process.ExitCode == 3) throw new InvalidDataException("This disc image isn't Marvel Ultimate Alliance 2 for Wii (USA, RMSE52).\r\n" + image);
            if (process.ExitCode == 2) throw new InvalidDataException("This file isn't a readable Wii disc image.\r\n" + image);
            if (process.ExitCode != 0) throw new InvalidOperationException("Extracting the disc failed (" + process.ExitCode + ").\r\n" + errors.ToString().Trim());
        }
    }

    static void EnsureGameData(string cache, string root, string game)
    {
        var manifest = ContentManifest();
        string wad = Path.Combine(game, "files", "z", "assetsfb.wad");
        bool extracted = File.Exists(Path.Combine(game, "sys", "main.dol"));
        if (manifest == null || (extracted && ContentInstalled(wad, manifest))) return;
        string image = extracted ? null : FindDiscImage(root);
        string staging = Path.Combine(root, "GameData.setup");

        var form = new Form();
        form.Text = "OpenMUA2 setup";
        form.FormBorderStyle = FormBorderStyle.FixedDialog;
        form.MaximizeBox = form.MinimizeBox = false;
        form.StartPosition = FormStartPosition.CenterScreen;
        form.ClientSize = new System.Drawing.Size(480, 132);
        try { form.Icon = System.Drawing.Icon.ExtractAssociatedIcon(Assembly.GetExecutingAssembly().Location); } catch { }
        var heading = new Label { Left = 16, Top = 14, Width = 448, Height = 20,
            Text = extracted ? "Adding OpenMUA2 content to your game data" : "Setting up Marvel Ultimate Alliance 2 from your disc" };
        heading.Font = new System.Drawing.Font(heading.Font, System.Drawing.FontStyle.Bold);
        var status = new Label { Left = 16, Top = 40, Width = 448, Height = 20, Text = "Starting" };
        var bar = new ProgressBar { Left = 16, Top = 64, Width = 448, Height = 20, Maximum = 1000 };
        var cancel = new Button { Text = "Cancel", Left = 384, Top = 96, Width = 80 };
        form.Controls.AddRange(new Control[] { heading, status, bar, cancel });

        Exception failure = null;
        bool canceled = false, finished = false;
        Process running = null;
        object gate = new object();
        Func<Process, bool> track = delegate(Process p) { lock (gate) { running = p; return !canceled; } };
        double phaseStart = 0, phaseSpan = 1;
        Action<string, double> report = delegate(string text, double fraction) {
            lock (gate) if (canceled) throw new OperationCanceledException("Setup was canceled.");
            int value = (int)(1000 * (phaseStart + phaseSpan * fraction));
            form.BeginInvoke((Action)delegate {
                if (text != null) status.Text = text;
                bar.Value = Math.Max(0, Math.Min(1000, value));
            });
        };
        cancel.Click += delegate { form.Close(); };
        form.FormClosing += delegate(object sender, FormClosingEventArgs e) {
            if (finished) return;
            lock (gate) {
                canceled = true;
                if (running != null) try { running.Kill(); } catch { }
            }
            status.Text = "Canceling";
            cancel.Enabled = false;
            e.Cancel = true; // The worker closes the window once it has stopped.
        };
        var worker = new Thread(delegate() {
            try {
                if (!extracted) {
                    if (Directory.Exists(staging)) Directory.Delete(staging, true);
                    phaseSpan = 0.6;
                    report("Extracting " + Path.GetFileName(image), 0);
                    ExtractDisc(cache, image, staging, report, track);
                    phaseStart = 0.6; phaseSpan = 0.4;
                    InstallContent(staging, manifest, report);
                    if (Directory.Exists(game)) Directory.Delete(game, false); // Only an empty leftover folder.
                    Directory.Move(staging, game);
                } else {
                    InstallContent(game, manifest, report);
                }
            } catch (Exception e) { failure = e; }
            if (failure != null) try {
                if (Directory.Exists(staging)) Directory.Delete(staging, true);
                if (File.Exists(wad + ".pending")) File.Delete(wad + ".pending");
            } catch { }
            form.BeginInvoke((Action)delegate { finished = true; form.Close(); });
        });
        worker.IsBackground = true;
        form.Shown += delegate { worker.Start(); };
        Application.EnableVisualStyles();
        Application.Run(form);
        worker.Join();
        if (canceled) throw new OperationCanceledException("Setup was canceled. Open OpenMUA2 again to finish setting up.");
        if (failure != null) throw failure;
    }

    // Build check: every custom member in content.zip matches its manifest hash.
    static int VerifyContent()
    {
        var manifest = ContentManifest();
        if (manifest == null) return 0;
        var custom = new Dictionary<string, string>(StringComparer.Ordinal);
        foreach (var m in manifest) if (m.Custom) custom.Add(m.Name, m.Sha);
        using (var s = Assembly.GetExecutingAssembly().GetManifestResourceStream("content.zip"))
        using (var zip = new ZipArchive(s, ZipArchiveMode.Read))
        {
            if (zip.Entries.Count != custom.Count) throw new InvalidDataException("Content archive count mismatch.");
            foreach (var entry in zip.Entries) {
                string expected;
                if (!custom.TryGetValue(entry.FullName, out expected)) throw new InvalidDataException("Unexpected content member: " + entry.FullName);
                using (var data = entry.Open()) if (Hash(data) != expected) throw new InvalidDataException("Content checksum failed: " + entry.FullName);
            }
        }
        return custom.Count;
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
        // Keyboard and mouse play needs no gamepad. Player one defaults to the
        // first Xbox slot, so a controller connected later works as usual.
        if (String.IsNullOrEmpty(device)) device = "XInput/0/Gamepad";
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
        var startup = Stopwatch.StartNew();
        bool verify = args.Length == 2 && args[0] == "--verify-package";
        try
        {
            if (args.Length != 0 && !verify) throw new ArgumentException("Unsupported launcher arguments.");
            var self = Process.GetCurrentProcess();
            if ((self.ProcessorAffinity.ToInt64() & 4) == 0) throw new InvalidOperationException("Logical CPU 2 is unavailable.");
            self.ProcessorAffinity = new IntPtr(4);
            // Package verification is a self-contained build check. Keep its
            // extraction beside the requested report instead of in user AppData.
            if (verify) local = Path.Combine(Path.GetDirectoryName(Path.GetFullPath(args[1])), "package-verification");
            string cache = Prepare();
            long packageMilliseconds = startup.ElapsedMilliseconds;
            if (verify)
            {
                string report = Path.GetFullPath(args[1]);
                int result = Run(StartInfo(cache, "--help"), report + ".runner.log");
                if (result != 0) throw new InvalidOperationException("Packaged runner help failed: " + result);
                string help = File.ReadAllText(report + ".runner.log");
                if (!help.Contains("--game") || !help.Contains("--user-dir")) throw new InvalidDataException("Runner help was incomplete.");
                int content = VerifyContent();
                File.WriteAllText(report, "PASS: All embedded runtime files verified against SHA256 manifest.\r\nContent members verified: " + content + "\r\nRunner --help exit: 0\r\nAffinity mask: " + self.ProcessorAffinity.ToInt64() + "\r\nRuntime cache: " + cache + "\r\nSaves path: " + Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "saves") + "\r\nGameplay was not launched. Visuals, audio and controller behavior were not tested.\r\n");
                return 0;
            }
            string root = AppDomain.CurrentDomain.BaseDirectory;
            string game = Path.Combine(root, "GameData");
            EnsureGameData(cache, root, game);
            if (!File.Exists(Path.Combine(game, "sys", "main.dol"))) throw new FileNotFoundException("GameData is missing. Keep GameData next to OpenMUA2.exe.");
            string saves = Path.Combine(root, "saves");
            Directory.CreateDirectory(saves);
            string logs = Path.Combine(local, "Logs");
            Directory.CreateDirectory(logs);
            string session = DateTime.Now.ToString("yyyyMMdd-HHmmss-fff");
            string log = Path.Combine(root, "OpenMUA2.log");
            if (File.Exists(log)) File.Copy(log, Path.Combine(logs, "previous-runtime.log"), true);
            EnsureControls(cache, saves, Path.Combine(logs, session + "-controller.log"));
            var info = StartInfo(cache, "--game " + Quote(game) + " --cpu jit --title OpenMUA2 --user-dir " + Quote(saves) + " --graphics Vulkan --audio Cubeb");
            ConfigureXboxUi(info, game);
            startupReport = "[startup] launcher preparation: " + startup.ElapsedMilliseconds +
                " ms (package verification: " + packageMilliseconds + " ms)";
            info.EnvironmentVariables["MODERNGEKKO_FRAME_TIMES"] = Path.Combine(logs, session + "-frames.csv");
            int exit = Run(info, log);
            if (exit != 0) MessageBox.Show("OpenMUA2 exited with code " + exit + ".\r\nDetails: " + log, "OpenMUA2", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return exit;
        }
        catch (OperationCanceledException e)
        {
            if (verify) File.WriteAllText(Path.GetFullPath(args[1]), "FAIL: " + e.ToString());
            else MessageBox.Show(e.Message, "OpenMUA2", MessageBoxButtons.OK, MessageBoxIcon.Information);
            return 1;
        }
        catch (Exception e)
        {
            if (verify) File.WriteAllText(Path.GetFullPath(args[1]), "FAIL: " + e.ToString());
            else MessageBox.Show("Unable to start OpenMUA2.\r\n" + e.Message, "OpenMUA2", MessageBoxButtons.OK, MessageBoxIcon.Error);
            return 1;
        }
    }
}
