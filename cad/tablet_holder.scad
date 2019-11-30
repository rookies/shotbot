difference() {
    cube([32,32,36], center=true);
    translate([-1,-1,0]) cube([30,30,32], center=true);
    translate([0,0,18]) cylinder(d=10, h=5, center=true);
    translate([0,0,-18]) cylinder(d=3, h=5, center=true, $fn=20);
}