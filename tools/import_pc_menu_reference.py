"""Copy the declared OpenXML1 PC menu reference into OpenMUA2's private workspace.

This is an authoring input, not an installable MUA2 package. Preserve original
paths and record hashes; do not copy unrelated game data or publish assets.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-repo', type=Path, required=True)
    parser.add_argument('--assets', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    repository = Path(__file__).resolve().parents[1]
    reference = args.reference_repo.resolve(strict=True)
    assets = args.assets.resolve(strict=True)
    output = args.output.resolve()
    if not assets.is_relative_to(reference):
        raise ValueError('Reference assets must be inside the reference repository')
    if not output.is_relative_to((repository / '.local').resolve()) or output.exists():
        raise ValueError('Output must be a new directory inside this repository/.local')

    # Use the reference project's existing package reader without creating a
    # Python cache or changing its source. All output belongs to OpenMUA2.
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location(
        'xml1_package_reference', reference / 'scripts/xml1_packages.py')
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    paths = set()
    declarations = {}
    for menu in ('options', 'options_controller_xbox'):
        package = Path('packages/generated/maps/package/menus') / (menu + '.pkgb')
        paths.add(package)
        declarations[package.as_posix()] = []
        for row in module.read_pkgb((assets / package).read_bytes()):
            fields = dict(row.attributes)
            base = Path(fields['filename'])
            if base.is_absolute() or '..' in base.parts:
                raise ValueError('Unsafe resource path')
            if row.kind not in ('model', 'xml'):
                raise ValueError(f'Unsupported resource type: {row.kind}')
            suffixes = ('.igb',) if row.kind == 'model' else ('.eng',)
            for suffix in suffixes:
                paths.add(Path(str(base) + suffix))
            declarations[package.as_posix()].append({'kind': row.kind, **fields})

    # Validate every dependency before creating output. Never follow a resource
    # junction/symlink outside the explicitly authorized reference tree.
    payloads = {}
    for relative in sorted(paths):
        source = (assets / relative).resolve(strict=True)
        if not source.is_relative_to(reference):
            raise ValueError(f'Resource escapes reference repository: {relative}')
        payloads[relative] = source.read_bytes()
    manifest = {'purpose': 'Private native menu authoring reference; not staged',
                'source': str(assets), 'packages': declarations, 'files': []}
    for relative, data in payloads.items():
        destination = output / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
        manifest['files'].append({'path': relative.as_posix(), 'bytes': len(data),
                                  'sha256': hashlib.sha256(data).hexdigest()})
    (output / 'reference-manifest.json').write_text(
        json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(f'Copied {len(payloads)} declared files, '
          f'{sum(map(len, payloads.values())):,} bytes into {output}')


if __name__ == '__main__':
    main()
