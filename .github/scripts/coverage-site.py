#!/usr/bin/env python3
"""Builds the coverage site from an llvm-cov JSON summary and its HTML report.

The site is the page in .github/coverage-site, a data.json describing this
report (totals, modules and files), a history.json with one entry per push to
main, and the llvm-cov HTML report under details/.
"""

import argparse
import json
import shutil
from datetime import datetime, timezone
from pathlib import Path

METRICS = ("lines", "functions", "regions", "branches")
HISTORY_LIMIT = 500


def parse_arguments():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--summary", required=True, type=Path,
                        help="llvm-cov export -summary-only output")
    parser.add_argument("--report", required=True, type=Path,
                        help="llvm-cov HTML report directory")
    parser.add_argument("--history", required=True, type=Path,
                        help="history.json currently published, may be missing")
    parser.add_argument("--template", required=True, type=Path,
                        help="directory holding the page")
    parser.add_argument("--site", required=True, type=Path,
                        help="output directory")
    parser.add_argument("--commit", required=True)
    parser.add_argument("--branch", required=True)
    parser.add_argument("--event", required=True)
    parser.add_argument("--pull-request", default="")
    parser.add_argument("--repository-url", default="")
    parser.add_argument("--run-url", default="")
    parser.add_argument("--append-history", action="store_true",
                        help="add this report to the history")
    return parser.parse_args()


def counts(summary):
    return {metric: {"covered": summary[metric]["covered"],
                     "count": summary[metric]["count"]}
            for metric in METRICS}


def relative_path(filename):
    parts = Path(filename).parts
    for start in range(1, len(parts)):
        candidate = Path(*parts[start:])
        if candidate.exists():
            return candidate.as_posix()
    return filename.lstrip("/")


def module_of(path):
    parts = path.split("/")
    if len(parts) < 3 or parts[1] in ("include", "src"):
        return parts[0]
    return f"{parts[0]}/{parts[1]}"


def describe_files(export):
    files = []
    for entry in export["files"]:
        path = relative_path(entry["filename"])
        files.append({
            "path": path,
            "module": module_of(path),
            "link": f"details/coverage{entry['filename']}.html",
            **counts(entry["summary"]),
        })
    return sorted(files, key=lambda file: file["path"])


def describe_modules(files):
    modules = {}
    for file in files:
        module = modules.setdefault(file["module"], {
            "name": file["module"],
            "files": 0,
            **{metric: {"covered": 0, "count": 0} for metric in METRICS},
        })
        module["files"] += 1
        for metric in METRICS:
            module[metric]["covered"] += file[metric]["covered"]
            module[metric]["count"] += file[metric]["count"]
    return sorted(modules.values(), key=lambda module: module["name"])


def load_history(path):
    try:
        history = json.loads(path.read_text())
    except (OSError, ValueError):
        return []
    return history if isinstance(history, list) else []


def main():
    arguments = parse_arguments()
    export = json.loads(arguments.summary.read_text())["data"][0]
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    totals = counts(export["totals"])
    files = describe_files(export)

    data = {
        "generated": now,
        "commit": arguments.commit,
        "branch": arguments.branch,
        "event": arguments.event,
        "pullRequest": arguments.pull_request,
        "repositoryUrl": arguments.repository_url,
        "runUrl": arguments.run_url,
        "totals": totals,
        "modules": describe_modules(files),
        "files": files,
    }

    history = load_history(arguments.history)
    if arguments.append_history:
        history = [entry for entry in history
                   if entry.get("commit") != arguments.commit]
        history.append({"date": now, "commit": arguments.commit,
                        "totals": totals})
        history = history[-HISTORY_LIMIT:]

    if arguments.site.exists():
        shutil.rmtree(arguments.site)
    shutil.copytree(arguments.template, arguments.site)
    shutil.copytree(arguments.report, arguments.site / "details")
    (arguments.site / ".nojekyll").touch()
    (arguments.site / "data.json").write_text(json.dumps(data))
    (arguments.site / "history.json").write_text(json.dumps(history))

    print(f"{len(files)} files, {len(data['modules'])} modules, "
          f"{len(history)} history entries")
    for metric in METRICS:
        total = totals[metric]
        share = 100 * total["covered"] / total["count"] if total["count"] else 0
        print(f"{metric}: {share:.1f}% ({total['covered']}/{total['count']})")


if __name__ == "__main__":
    main()
