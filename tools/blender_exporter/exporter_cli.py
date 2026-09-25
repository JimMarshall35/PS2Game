import sys
import argparse
import os

# Blender doesn't reliably put the script's own dir on sys.path
script_dir = os.path.dirname(os.path.abspath(__file__))
if script_dir not in sys.path:
    sys.path.insert(0, script_dir)

from exporter import do_export

def main():
    # Blender puts its own args before `--`; strip everything up to and including it
    argv = sys.argv
    if "--" in argv:
        argv = argv[argv.index("--") + 1:]
    else:
        argv = []

    parser = argparse.ArgumentParser()
    parser.add_argument("--pak_manifest", default="./out/pak.json")
    parser.add_argument("--platform", default="PC")
    parser.add_argument("--output_directory", default="./out")

    args = parser.parse_args(argv)
    do_export(
        context=None,
        directory=args.output_directory,
        platform=args.platform,
        pak_file_json_path=args.pak_manifest
    )
    print(f"Exported to {args.output_directory}")

if __name__ == "__main__":
    main()