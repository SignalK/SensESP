# CLAUDE.md — Fix `UIButton` in SensESP

This file briefs a fresh session on everything established so far about
`sensesp::UIButton` being non-functional, so that session can prepare a
real PR against `SensESP` (upstream: `SignalK/SensESP`, fork:
`BoatHacks/SensESP`) that makes it actually work.

Origin: this investigation happened while building
`BoatHacks/HALSER-HWT3100-interface`, an unrelated ESP32-C3 firmware
project that uses SensESP 3.5.0. That firmware is not otherwise
relevant here except as the source of the real-hardware confirmation
described below — don't go looking at it for SensESP-specific context,
there isn't any.

## The problem, in one line

`UIButton::add(name, title)` compiles, runs, and its doc comment says
it "creates a button in the Control tab of the web UI" — but nothing
happens. No button ever appears anywhere in the actual web UI, on any
page, and no callback ever fires. This was confirmed two ways:
static source analysis, and a real on-hardware test.

## Investigation log (what's already been established)

### 1. The class itself

`src/sensesp/ui/ui_button.h` / `ui_button.cpp`:

```cpp
class UIButton : public Observable {
 public:
  UIButton(String title, String name, bool must_confirm) ...
  static UIButton* add(String name, String title, bool must_confirm = true) {
    auto new_cmd = std::make_shared<UIButton>(title, name, must_confirm);
    ui_buttons_[name] = new_cmd;
    return new_cmd.get();
  }
  static const std::map<String, std::shared_ptr<UIButton>>& get_ui_buttons() { ... }
  ...
 protected:
  static std::map<String, std::shared_ptr<UIButton>> ui_buttons_;
};
```

`add()` self-registers into a static registry. Callers `attach()` a
`std::function<void()>` (via the `Observable` base class) that's meant
to fire on click (`notify()`). That's the entire class — no backend or
frontend code anywhere references `UIButton`, `ui_buttons_`, or
`get_ui_buttons()` except this file itself.

### 2. Backend: confirmed no route serves or consumes it

Checked every file in `src/sensesp/net/` (the HTTP server/handler
layer), in both the vendored 3.5.0 release and the current `main`
branch on GitHub:

- `config_handler.cpp` — only `/api/config*` (get/put persisted
  `ConfigItem`s via `FileSystemSaveable`). Nothing button-related.
- `app_command_handler.cpp` — WiFi scan + SignalK TOFU-reset endpoints
  only.
- `base_command_handler.cpp` — fixed, hardcoded endpoints:
  `/api/device/reset`, `/api/device/restart`, `/api/info`, `/api/log`,
  `/api/routes`. Nothing generic or pluggable for app-registered
  commands/buttons. This file also registers the actual page routes
  (`add_routes_handlers()`, see §4 below).

Grep command used (repeat this to re-verify against whatever commit
you're starting from):

```
grep -rln "UIButton\|ui_buttons\|get_ui_buttons" src/sensesp/net/
# -> no results, ever, in either version checked
```

### 3. Frontend: confirmed no rendering path exists either

The web UI actually served at `/` (and the other routes) is a
Vite/Preact/TypeScript SPA compiled into
`src/sensesp/net/web/autogen/frontend_files.h` (a generated
`kFrontendFiles[]` array of gzip-compressed static assets — the
*source* for this lives in the `frontend/` directory of the SensESP
repo itself, see §5).

The compiled JS was extracted directly out of a real build (decompress
the gzip blob for the `/assets/index-*.js` entry in
`kFrontendFiles[]`) and searched:

```
$ grep -o '/api/[a-zA-Z_/-]*' index_bundle.js | sort -u
/api/config
/api/device/reset
/api/device/restart
/api/info
/api/log
/api/routes
/api/signalk
/api/signalk/reset-tofu
/api/wifi/scan
/api/wifi/scan-results
```

No `/api/command`, `/api/buttons`, or anything resembling a generic
custom-action endpoint. The only two buttons anywhere in the real,
current UI are hardcoded ones on the System page — "Restart the
device" and "Reset to factory defaults" — wired to the two fixed
`/api/device/*` endpoints above. There's no code path that reads a
dynamic list of registered commands/buttons and renders one per entry.

### 4. There IS a `pages/` directory — and there's no "Control" page in it

`frontend/src/pages/` (in the SensESP repo, not the compiled output)
currently contains exactly: `Configuration/`, `Home/`, `Log/`,
`SignalK/`, `Status/`, `System/`, `WiFi/`, plus `AppPage.tsx`,
`PageContents.tsx`, `PageHeading.tsx`, `Root.tsx`, `_404.tsx`.

This matches **exactly** the routes registered server-side in
`base_command_handler.cpp`'s `add_routes_handlers()`:

```cpp
routes.push_back(RouteDefinition("Status", "/status", "StatusPage"));
routes.push_back(RouteDefinition("System", "/system", "SystemPage"));
routes.push_back(RouteDefinition("Log", "/log", "LogPage"));
routes.push_back(RouteDefinition("WiFi", "/wifi", "WiFiConfigPage"));
routes.push_back(RouteDefinition("Signal K", "/signalk", "SignalKPage"));
routes.push_back(RouteDefinition("Configuration", "/configuration", "ConfigurationPage"));
```

**There is no "Control" page anywhere in the current frontend.** This
means the fix isn't "reconnect an existing-but-disconnected page" — a
Control page needs to be built from scratch: new route, new page
component, a way to render a dynamic list of buttons, plus the new
backend endpoint(s) to back it.

### 5. A vestigial, already-dead predecessor exists (useful context, not a shortcut)

A second, older, separate frontend is *also* still compiled in:
`src/sensesp/net/web/js_sensesp.h` (`index.h`, `css_bootstrap.h`,
`static_file_data.h`, served via `add_static_file_handlers()`, called
alongside the newer `add_routes_handlers()` in `sensesp_app.h`).
Decompiling that older bundle shows a `showControl()` function:

```js
function showControl(){
  ...
  ajax("GET","/command?id="+name)...
}
```

...referencing `deviceInfo.Commands` (an array with `.Name`, `.Title`,
`.Confirm` fields — note the shape matches `UIButton`'s
`name_`/`title_`/`must_confirm_` almost exactly). This is dead too:
nothing populates a `Commands` key in any `/info`-family response, and
nothing handles `GET /command`. It's not served at the default routes
either (the newer `autogen/frontend_files.h` bundle is what's actually
registered for `/`, `/status`, etc.).

**Working theory, not confirmed**: `UIButton` is very likely the
surviving C++ half of this older "Control" page concept. When the
frontend was rewritten as the current Vite/Preact SPA (dropping the
Control page entirely, judging by its absence from `pages/`), the
matching frontend/backend wiring was never rebuilt, but the `UIButton`
class itself was never removed either. Worth checking `git log` /
`git blame` on `ui_button.h` and the old `js_sensesp` frontend for
when each was touched, to confirm the timeline — this session did not
do that.

### 6. Confirmed on real hardware, not just by static analysis

A HALSER-HWT3100-interface firmware build added a real `UIButton`
whose callback fired an `HTTPClient` GET to a SignalK server (a
side-effect observable independently of the web UI, via the SignalK
server's own access log, to rule out "maybe it's rendered somewhere I
didn't check"). Flashed to real hardware; every page of the actual
served web UI (Status, System, Log, WiFi, SignalK, Configuration) was
checked by hand. **The button never appeared anywhere.** No request
ever reached the SignalK server either. This matches the static
analysis exactly and is about as conclusive as this can get without
already having the fix.

## Reference material found for a `ButtonCard`-style pattern to reuse

`frontend/src/pages/System/index.tsx` (~450 lines) is the closest
existing analog to what a Control page needs: a page with several
"cards," each posting to a backend endpoint on click, with toast
feedback and (for Reset) a danger/confirmation style. Structure
(per an AI-assisted read of the file — **verify directly**, this
wasn't fetched in full):

- `SystemPage` — page wrapper (routing target)
- `SystemCards` — container rendering the individual cards
- `DeviceNameCard`, `AuthCard`, `RestartCard`, `ResetCard` — one per
  setting/action
- `RestartCard`/`ResetCard` POST to `/api/device/restart` /
  `/api/device/reset` respectively; on success, toast + reload; on
  error, a danger-colored toast
- Uses a `ButtonCard` component — **not** present in
  `frontend/src/components/` (which only has `Card.tsx`, `Collapse.tsx`,
  `Form.tsx`, `Header.tsx`, `Tab.tsx`, `ToastMessage.tsx`,
  `useToast.ts`) — so `ButtonCard` is most likely defined locally
  inside `System/index.tsx`, built on top of the generic `Card.tsx`,
  not already extracted as a shared component. A Control page would
  probably want to extract/generalize it into `components/` rather
  than duplicate it.
- Imports Preact hooks (`useState`, `useEffect`, `useContext`,
  `useId`), Immer for immutable state updates, and a config-API client
  for fetching/saving settings.

## What a real fix looks like (sketch, not a finished design)

1. **Backend**: a new HTTP handler (own file under
   `src/sensesp/net/web/`, following the existing
   `app_command_handler.cpp`/`base_command_handler.cpp` pattern) that:
   - Serves `GET /api/buttons` (or similar) — JSON array from
     `UIButton::get_ui_buttons()`: `name`, `title`, `must_confirm`.
   - Serves `POST /api/buttons/<name>` (or `?id=<name>`, matching the
     old dead `/command?id=` shape if that naming is worth keeping) —
     looks up the button by name and calls `notify()` on it.
   - Register it in `sensesp_app.h` alongside the other
     `add_*_http_command_handlers()` calls.
2. **Frontend**: a new `pages/Control/index.tsx` (or similar) that:
   - Fetches `/api/buttons` on mount.
   - Renders one button/card per entry (reusing or extracting the
     `ButtonCard` pattern from `System/index.tsx`), respecting
     `must_confirm` (the System page's Reset card already has a
     confirm-styled pattern to copy).
   - POSTs to `/api/buttons/<name>` on click, with toast
     success/error feedback matching the existing System page's
     style.
   - Register the new route in both places routes are currently
     declared: `base_command_handler.cpp`'s `add_routes_handlers()`
     (`RouteDefinition("Control", "/control", "ControlPage")` or
     similar) and the frontend's own routing (`App.tsx`).
3. **Build**: `frontend/` uses pnpm + Vite
   (`frontend/package.json`, `frontend/vite.config.ts`). After
   frontend changes, the compiled output needs to be regenerated into
   `src/sensesp/net/web/autogen/frontend_files.h` — find and follow
   whatever build/codegen script SensESP's own `frontend/README.md` or
   root build tooling documents for this step (not yet located/read in
   this session — check there first).
4. **Test**: build against a real ESP32 target with a `UIButton`
   registered (the temporary probe pattern from
   `HALSER-HWT3100-interface`'s `docs/plans/uibutton-investigation.md`
   is a reusable template: register a button whose callback has an
   independently-observable side effect, e.g. an HTTP request to
   somewhere with visible logs, so success/failure doesn't depend on
   trusting the UI alone).

## Open items for the next session

- Locate and read the actual frontend build/codegen instructions
  (`frontend/README.md`, root `README.md`, or CI workflow files) to
  confirm exactly how `frontend_files.h` gets regenerated from
  `frontend/`.
- Read `App.tsx` and the config-API client (`frontend/src/common/` —
  contents not yet examined) to nail down the exact routing
  registration and fetch/POST idioms before writing new code, rather
  than guessing from the System page summary alone.
- Confirm the `git blame`/history theory in §5 (optional — context,
  not blocking).
- Decide on the actual `/api/buttons*` endpoint shape (this doc
  proposes one; it hasn't been validated against SensESP maintainer
  preferences — worth a lightweight check/issue comment before
  investing in a full implementation, since this is someone else's
  project).
- `BoatHacks/SensESP` (the fork intended for this PR) was not
  attached to the session that produced this file — repeated
  `add_repo` attempts failed with `MCP tool call requires approval`
  and were never resolved. The next session will need working repo
  access (clone/push) before it can do anything beyond further
  read-only investigation via raw file fetches.
