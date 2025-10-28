/* SSH for Github, Step 1 — Check for existing SSH keys: <p>Run <code>ls -al ~/.ssh</code>. If you see files like <code>id_ed25519</code> and <code>id_ed25519.pub</code> you already have a key pair; the <code>.pub</code> file is the public key. If not, generate a key pair in the next step.</p> */

/* SSH for Github, Step 2 — Generate an Ed25519 SSH key pair: <p>Run <code>ssh-keygen -t ed25519 -C "your_email@example.com
"</code>. Accept the default file path by pressing Enter (creates <code>~/.ssh/id_ed25519</code> and <code>~/.ssh/id_ed25519.pub</code>). Optionally add a passphrase for extra security. Ed25519 is modern, short, and secure.</p> */

/* SSH for Github, Step 3 — Start the SSH agent and add your private key: <p>Start the agent: <code>eval "$(ssh-agent -s)"</code>. Then add your key: <code>ssh-add ~/.ssh/id_ed25519</code>. The agent holds the unlocked key in memory so you don't retype the passphrase each SSH session.</p> */

/* SSH for Github, Step 4 — Copy the public key to GitHub: <p>Show the public key with <code>cat ~/.ssh/id_ed25519.pub</code> and copy the entire single-line string starting with <code>ssh-ed25519</code>. In GitHub go to <strong>Settings → SSH and GPG keys → New SSH key</strong>, paste the key, give it a descriptive title, and save. This tells GitHub to trust signatures from your private key.</p> */

/* SSH for Github, Step 5 — Test GitHub SSH authentication: <p>Run <code>ssh -T git@github.com
</code>. Expected reply: <em>Hi <username>! You've successfully authenticated, but GitHub does not provide shell access.</em> That confirms authentication is working.</p> */

/* SSH for Github, Step 6 — Use SSH URLs for Git operations: <p>Clone repos with the SSH form: <code>git clone git@github.com
:username/repo.git</code>. To switch an existing repo from HTTPS to SSH: <code>git remote set-url origin git@github.com
:username/repo.git</code>. This avoids HTTPS credential prompts.</p> */

/* SSH for Github, Step 7 — Useful GitHub SSH troubleshooting checklist: <ol><li><p>Ensure the public key on GitHub exactly matches <code>~/.ssh/id_ed25519.pub</code>.</p></li><li><p>Check <code>ssh-add -l</code> lists your key (agent has it).</p></li><li><p>Verbose SSH output: <code>ssh -vT git@github.com
</code> shows which key is used and any errors.</p></li><li><p>File permissions: <code>chmod 700 ~/.ssh</code> and <code>chmod 600 ~/.ssh/id_ed25519</code>.</p></li></ol> */

/* SSH for Github, Key-points summary — GitHub SSH: <ul><li><p>Public key on GitHub = lock; private key on your machine = key.</p></li><li><p>SSH agent caches unlocked private key for sessions.</p></li><li><p>Use SSH remote URLs so git uses your key-based auth automatically.</p></li></ul> */

/* SSH for MacBook, Step 1 — Goal & model: <p>Goal: set up SSH from your MacBook to your own server so you can log in securely without typing a password. Model: generate a key pair locally; put the public key in the server user's <code>~/.ssh/authorized_keys</code> with correct permissions.</p> */

/* SSH for MacBook, Step 2 — Check for existing keys on your Mac: <p>Run <code>ls -al ~/.ssh</code>. If keys exist (e.g. <code>id_ed25519</code> and <code>id_ed25519.pub</code>) you can reuse them; otherwise generate a new key in the next step.</p> */

/* SSH for MacBook, Step 3 — Generate an Ed25519 key pair on the Mac: <p>Run <code>ssh-keygen -t ed25519 -C "yourname@macbook"</code>. Accept default path (<code>~/.ssh/id_ed25519</code>). Optionally add a passphrase. This produces a private key (<code>id_ed25519</code>) and public key (<code>id_ed25519.pub</code>).</p> */

/* SSH for MacBook, Step 4 — Copy the public key to the server (easy method): <p>If password-based SSH is enabled on the server, run <code>ssh-copy-id username@server_ip</code>. This appends your public key to the server's <code>~/.ssh/authorized_keys</code> and sets correct permissions.</p> */

/* SSH for MacBook, Step 5 — Copy the public key to the server (manual fallback): <p>If <code>ssh-copy-id</code> is not available, run:</p><pre><code>cat ~/.ssh/id_ed25519.pub | ssh username@server_ip "mkdir -p ~/.ssh && chmod 700 ~/.ssh && cat >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys"</code></pre><p>This creates <code>~/.ssh</code> if missing, appends the key, and sets secure permissions (critical for SSH to accept keys).</p> */

/* SSH for MacBook, Step 6 — Test the SSH login: <p>Now run <code>ssh username@server_ip</code>. If keys and permissions are correct you will log in without a password. If it still asks for a password, check <code>~/.ssh/authorized_keys</code> on the server contains the exact public-key line and permissions (<code>700</code> for <code>~/.ssh</code>, <code>600</code> for <code>authorized_keys</code>).</p> */

/* SSH for MacBook, Step 7 — Debugging remote key problems: <ol><li><p>On the client, run <code>ssh -v username@server_ip</code> to see which key is offered and server responses.</p></li><li><p>On the server, inspect <code>/var/log/auth.log</code> (or <code>/var/log/secure</code>) for SSHD messages about key acceptance.</p></li><li><p>Make sure server's <code>/etc/ssh/sshd_config</code> allows <code>PubkeyAuthentication yes</code> and that <code>PasswordAuthentication</code> is <code>yes</code> only during setup; you can disable it later for hardening.</p></li></ol> */

/* SSH for MacBook, Step 8 — Optional: Create an SSH config entry for convenience: <p>Edit <code>~/.ssh/config</code> (create if missing) and add:</p><pre><code>Host myserver
HostName 203.0.113.45
User username
IdentityFile ~/.ssh/id_ed25519
IdentitiesOnly yes</code></pre><p>Then connect simply with <code>ssh myserver</code>. <code>IdentitiesOnly yes</code> ensures the client presents only the specified key, avoiding server rejections due to too many offered keys.</p> */

/* SSH for MacBook, Step 9 — Optional: Disable password authentication (hardening): <p>After confirming key logins work, on the server edit <code>/etc/ssh/sshd_config</code> and set <code>PasswordAuthentication no</code> and <code>ChallengeResponseAuthentication no</code>, then restart SSH (<code>sudo systemctl restart sshd</code>). This prevents password-based logins; keep a fallback access method (console, cloud provider serial) in case keys fail.</p> */

/* SSH for MacBook, Key-points summary — Mac → Server SSH: <ul><li><p>Private key stays on Mac; public key goes in the server's <code>~/.ssh/authorized_keys</code>.</p></li><li><p>Correct file permissions are mandatory: <code>~/.ssh</code> = <code>700</code>, <code>authorized_keys</code> = <code>600</code>.</p></li><li><p>Use <code>ssh -v</code> to debug; check server logs if needed.</p></li><li><p>Only disable password auth after verifying key-based logins and ensuring a recovery path.</p></li></ul> */