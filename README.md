# app-template

Template for an esp-claw WASM app: one example tool (`myapp_greet`) that parses its arguments with `claw_util.h`. It comes with the [app-sdk](https://github.com/esp-claw/app-sdk) submodule and a CI workflow that builds and checks every push and attaches the build to releases.

Part of [esp-claw (community)](https://github.com/esp-claw), an unofficial community fork of espressif/esp-claw. It is not affiliated with Espressif.

## Use this template

1. Click **Use this template** on GitHub, or run `gh repo create <owner>/app-<something> --template esp-claw/app-template`. Then clone the new repository with its submodule:

   ```sh
   git clone --recursive https://github.com/<owner>/app-<something>.git
   ```

2. **Name the app.** Set `name` in `manifest.json`. The name is 1-24 chars of `[a-z0-9_]` and starts with a letter. It does two things:
   - The device keeps the app in `apps/<name>/`. A copied app loads only when its directory name equals the manifest name. `/app install` takes care of that.
   - The name prefixes every tool. A tool registered as `greet` reaches the LLM as `<name>_greet`, and the full tool name is limited to 48 chars, so keep the name short.

   The repository name does not have to match. The esp-claw apps use `app-<long name>` with a short app name, for example `app-cloudflare` with `cf`. Also change the `myapp` mentions in `main.c` and this README.

3. **Write the tools** in `main.c`. Register each tool in `app_init()` with a JSON Schema, and handle it in `tool_call()`. `sdk/include/claw.h` documents the host API. Only use host functions whose permission you grant in `manifest.json` (`log`, `kv`, `confirm`, `ping`, `http_profiles`), and grant only what the app uses. Adjust `limits.memory_kb` and `limits.call_timeout_ms` if the app needs more memory or time.

4. **Build and check:**

   ```sh
   ./sdk/build-app.sh .                          # -> build/app.wasm, build/manifest.json
   python3 sdk/tools/manifest_check.py .
   python3 sdk/tools/wasm_check.py build
   ```

   The build needs clang with the wasm32 target and wasm-ld (Debian/Ubuntu: `apt install clang lld`). CI runs the same steps through the reusable workflow `esp-claw/app-sdk/.github/workflows/app-build.yml@v1`.

5. **Try it on a development device.** Set `app_allow_unsigned` to `true` in the device config and reboot. Then copy `build/manifest.json` and `build/app.wasm` to `/apps/<name>/`, for example over WebDAV, and send `/apps reload <name>` in Telegram.

## Releases

Bump `version` in `manifest.json` (semver), then publish a release tagged `v<version>`. CI attaches `app.wasm` and `manifest.json` to the release. It refuses to do so if the tag does not match the manifest version. Then sign the CI build locally and upload the signed package:

```sh
v=0.1.0
gh release download "v$v" -p app.wasm -p manifest.json -D dist
python3 sdk/tools/ecapp.py pubkey /path/to/key.pem          # base64 key for app_trusted_keys
python3 sdk/tools/ecapp.py pack dist --key /path/to/key.pem # -> dist/<name>-$v.ecapp
gh release upload "v$v" dist/<name>-$v.ecapp
```

Create a key once with `openssl genpkey -algorithm ed25519 -out key.pem` and keep it out of the repository. The [app-sdk README](https://github.com/esp-claw/app-sdk#packages-and-signing) describes the package format and trusted keys.

## Install on the device

The device must trust your public key: add it to `app_trusted_keys` in the device config and reboot. Then send in Telegram:

```
/app install https://github.com/<owner>/<repo>/releases/download/v<version>/<name>-<version>.ecapp
```

The device asks for confirmation before installing. To update, send `/app update <name> <url of the newer .ecapp>`. `/app remove <name>` deletes the app.

## License

Apache-2.0
