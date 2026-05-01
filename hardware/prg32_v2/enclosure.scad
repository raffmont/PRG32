// PRG32 v2 enclosure starter.
// Dimensions are conservative placeholders for classroom iteration.

case_w = 96;
case_h = 132;
case_d = 22;
wall = 2;

module rounded_box(w, h, d) {
    minkowski() {
        cube([w - 4, h - 4, d - 2], center = true);
        cylinder(r = 2, h = 2, center = true, $fn = 24);
    }
}

difference() {
    rounded_box(case_w, case_h, case_d);
    translate([0, 0, wall])
        rounded_box(case_w - wall * 2, case_h - wall * 2, case_d);

    // Display opening.
    translate([0, 18, case_d / 2])
        cube([72, 52, 4], center = true);

    // Button openings.
    for (x = [-24, 0, 24]) {
        translate([x, -44, case_d / 2])
            cylinder(r = 4, h = 4, center = true, $fn = 32);
    }
    for (x = [-12, 12]) {
        translate([x, -62, case_d / 2])
            cylinder(r = 4, h = 4, center = true, $fn = 32);
    }
}
