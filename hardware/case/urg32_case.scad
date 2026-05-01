$fn = 48;
module base() {
  difference() {
    cube([110, 80, 18], center=false);
    translate([3, 3, 3]) cube([104, 74, 16], center=false);
    translate([15, 8, 15]) cube([78, 52, 5], center=false);
  }
}
module button_holes() {
  translate([90, 58, 16]) cylinder(h=5, r=3);
  translate([100, 58, 16]) cylinder(h=5, r=3);
  translate([18, 58, 16]) cylinder(h=5, r=3);
  translate([28, 58, 16]) cylinder(h=5, r=3);
}
difference() { base(); button_holes(); }
