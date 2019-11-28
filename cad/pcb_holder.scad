G = 2.54; /* 2.54mm grid */
T = 3; /* thickness */
$fn = 20;

difference() {
    linear_extrude(T) difference() {
        minkowski() {
            square([34*G,28*G], center=true);
            circle(G);
        }
        /* machine screw holes: */
        for (i = [1,-1]) {
            for (j = [1,-1]) {
                translate([i*15*G,j*12*G])
                    circle(d=2.5);
            }
        }
        /* wood screw holes: */
        for (i = [1,-1]) {
            for (j = [1,-1]) {
                translate([i*10*G,j*7*G])
                    circle(d=2.5);
            }
        }
    }
    /* machine screw countersinks: */
    for (i = [1,-1]) {
        for (j = [1,-1]) {
            translate([i*15*G,j*12*G,-.5])
                cylinder(d=4, h=2);
        }
    }
    /* wood screw countersinks: */
    for (i = [1,-1]) {
        for (j = [1,-1]) {
            translate([i*10*G,j*7*G, 1])
                cylinder(d1=2.5, d2=4.5, h=2);
        }
    }
}