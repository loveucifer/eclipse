#!/usr/bin/env python3

import json
import shutil
import sys
from pathlib import Path


def fail(message: str) -> None:
    raise SystemExit(f"asset packaging failed: {message}")


def contained_file(root: Path, relative: str, asset_id: str) -> Path:
    path = Path(relative)
    if path.is_absolute():
        fail(f"{asset_id}: absolute paths are not allowed: {relative}")
    source = (root / path).resolve()
    try:
        source.relative_to(root)
    except ValueError:
        fail(f"{asset_id}: path escapes the asset root: {relative}")
    if not source.is_file():
        fail(f"{asset_id}: missing file: {relative}")
    return source


def main() -> None:
    if len(sys.argv) < 3:
        fail("usage: package_assets.py SOURCE_ROOT DESTINATION_ROOT [GROUP ...]")

    source_root = Path(sys.argv[1]).resolve()
    destination_root = Path(sys.argv[2]).resolve()
    requested_groups = set(sys.argv[3:])
    manifest_path = source_root / "manifest.json"
    if not manifest_path.is_file():
        fail(f"missing manifest: {manifest_path}")

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"cannot read manifest: {error}")

    if manifest.get("version") != 1 or not isinstance(manifest.get("assets"), list):
        fail("manifest must have version 1 and an assets array")

    sources: set[Path] = set()
    packaged_entries = []
    known_groups: set[str] = set()
    for entry in manifest["assets"]:
        if not isinstance(entry, dict):
            fail("every asset entry must be an object")
        asset_id = entry.get("id")
        asset_type = entry.get("type")
        if not isinstance(asset_id, str) or not asset_id:
            fail("every asset requires a non-empty id")
        if asset_type in ("texture", "audio"):
            fields = ("path",)
        elif asset_type == "font":
            fields = ("path",)
            if isinstance(entry.get("license"), str) and entry["license"]:
                fields += ("license",)
        elif asset_type == "shader":
            fields = ("vertex", "fragment")
        else:
            fail(f"{asset_id}: unsupported type: {asset_type}")

        group = entry.get("group")
        if isinstance(group, str):
            known_groups.add(group)
        selected = not requested_groups or group == "global" or group in requested_groups
        if selected:
            packaged_entries.append(entry)
        for field in fields:
            relative = entry.get(field)
            if not isinstance(relative, str) or not relative:
                fail(f"{asset_id}: missing {field}")
            source = contained_file(source_root, relative, asset_id)
            if selected:
                sources.add(source)

    unknown_groups = requested_groups - known_groups
    if unknown_groups:
        fail(f"unknown asset group(s): {', '.join(sorted(unknown_groups))}")

    if destination_root.exists():
        shutil.rmtree(destination_root)
    destination_root.mkdir(parents=True)
    packaged_manifest = {**manifest, "assets": packaged_entries}
    (destination_root / "manifest.json").write_text(
        json.dumps(packaged_manifest, indent=2) + "\n", encoding="utf-8"
    )
    for source in sorted(sources):
        relative = source.relative_to(source_root)
        destination = destination_root / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)

    groups = ", ".join(sorted(requested_groups)) if requested_groups else "all groups"
    print(f"Packaged {len(sources)} assets ({groups}) into {destination_root}")


if __name__ == "__main__":
    main()
