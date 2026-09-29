import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('generator',Path(__file__).parents[1]/'tools/generate_dol_mg01.py')
generator=importlib.util.module_from_spec(spec)
spec.loader.exec_module(generator)

class ChunkCountTest(unittest.TestCase):
    def test_partial_and_empty_text_sections(self):
        with tempfile.TemporaryDirectory() as temp:
            dol=Path(temp)/'main.dol'
            header=bytearray(256)
            struct.pack_into('>7I',header,0x90,4100,8192,0,0,0,0,0)
            dol.write_bytes(header)
            self.assertEqual(generator.expected_c_chunks(dol,4096),2)
            self.assertEqual(generator.expected_c_chunks(dol,1024),4)

if __name__=='__main__':unittest.main()
