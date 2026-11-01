#!/usr/bin/env python3
"""CI check of the APK's launcher entries (Settings > Look > App icon, AndroidManifest.xml): reads
`aapt2 dump xmltree --file AndroidManifest.xml <apk>` on stdin. The original icon's activity-alias
com.sheikahslate.SheikahSlateActivity is enabled with a MAIN / LAUNCHER filter; the other icons' aliases are there with
one and off; the game's activity, com.sheikahslate.GameActivity, has none of its own. (aapt2's `dump badging` doesn't
list aliases as launchable activities: it can't tell.) Exit 1 with the reason otherwise.
"""
import re
import sys

ORIGINAL = "com.sheikahslate.SheikahSlateActivity"
OTHERS = ["com.sheikahslate.IconSlate", "com.sheikahslate.IconMidnight", "com.sheikahslate.IconDawn", "com.sheikahslate.IconAncient",
          "com.sheikahslate.IconForest"]
GAME = "com.sheikahslate.GameActivity"


def parse(text):
    """The elements as (tag, attrs, children), by indentation."""
    root = ("", {}, [])
    stack = [(-1, root)]
    for line in text.splitlines():
        m = re.match(r"^( *)([EA]): (.*)$", line)
        if not m:
            continue
        depth, kind, rest = len(m.group(1)), m.group(2), m.group(3)
        while stack and stack[-1][0] >= depth:
            stack.pop()
        parent = stack[-1][1]
        if kind == "E":
            el = (rest.split(" ")[0], {}, [])
            parent[2].append(el)
            stack.append((depth, el))
        else:
            a = re.match(r"(?:\S+:)?(\w+)\(0x[0-9a-f]+\)=(.*)$", rest) or re.match(r"(?:\S+:)?(\w+)=(.*)$", rest)
            if a:
                val = a.group(2)
                q = re.match(r'"([^"]*)"', val)
                parent[1][a.group(1)] = q.group(1) if q else val.strip()
    return root


def walk(el):
    yield el
    for c in el[2]:
        yield from walk(c)


def launcher(el):
    names = {c[1].get("name") for c in walk(el) if c[0] in ("action", "category")}
    return "android.intent.action.MAIN" in names and "android.intent.category.LAUNCHER" in names


def enabled(el):
    v = el[1].get("enabled")
    if v is None:
        return True
    v = v.lower()
    return not (v == "false" or v.endswith("0x0") or v == "0")


def main():
    root = parse(sys.stdin.read())
    comps = {}
    for el in walk(root):
        if el[0] in ("activity", "activity-alias") and "name" in el[1]:
            comps[el[1]["name"]] = el
    errors = []
    o = comps.get(ORIGINAL)
    if not o or o[0] != "activity-alias":
        errors.append(f"no activity-alias {ORIGINAL}")
    elif not (launcher(o) and enabled(o)):
        errors.append(f"{ORIGINAL}: not an enabled launcher entry")
    for n in OTHERS:
        el = comps.get(n)
        if not el or el[0] != "activity-alias":
            errors.append(f"no activity-alias {n}")
        elif not launcher(el) or enabled(el):
            errors.append(f"{n}: should be a launcher entry, off")
    g = comps.get(GAME)
    if not g or g[0] != "activity":
        errors.append(f"no activity {GAME}")
    elif launcher(g):
        errors.append(f"{GAME} has a launcher filter of its own")
    for e in errors:
        print("launcher entries:", e)
    if errors:
        return 1
    print(f"launcher entries: {ORIGINAL} on, {len(OTHERS)} other icons off, {GAME} behind them")
    return 0


if __name__ == "__main__":
    sys.exit(main())
