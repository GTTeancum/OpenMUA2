"""Regressions from the original tab-strip metal, panel and LED colors."""
import unittest
from convert_pc_menu_assets import red_image


class PaletteTest(unittest.TestCase):
    def test_blue_gray_metal_stays_unchanged(self):
        for rgb in [(82, 105, 131), (82, 101, 123), (74, 93, 115), (127, 127, 127)]:
            self.assertEqual(red_image(rgb), rgb)

    def test_saturated_panels_and_bright_leds_turn_red(self):
        for rgb in [(0, 128, 255), (5, 20, 70), (130, 186, 255)]:
            r, g, b = red_image(rgb)
            self.assertGreater(r, g)
            self.assertEqual(g, b)
            self.assertEqual(r, max(rgb))

    def test_unrelated_colors_are_preserved(self):
        for rgb in [(255, 255, 255), (0, 0, 0), (255, 20, 0), (20, 255, 20)]:
            self.assertEqual(red_image(rgb), rgb)


if __name__ == '__main__':
    unittest.main()
