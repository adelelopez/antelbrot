ANTelbrot
=========

A Mandelbrot set viewer using the [perturbation theory](http://www.superfractalthing.co.nf/sft_maths.pdf) algorithm by K.I. Martin.

Commands:

ESC: close window

r: change the zoom radius

d: change the max iteration depth

i: change the center coordinates

z: zoom in on current location

left mouse click: zoom in at cursor location

![](http://i.imgur.com/BvpkZfY.jpg)

Notes:

* The perturbation theory algorithm requires a point with a high iteration depth to work properly. Automatically selecting a suitble point will be implemented in a future version. For now, select points within the Mandelbrot set for best results.

* By default, this is compiled with the gcc flag -Ofast - which uses unsafe floating point arithmetic. The perturbation theory algorithm is designed to be less sensitive to precision, so I expect this to not be an issue. In practice, there is not an apparent difference in the images, while the rendering time is *much* faster. Calculations that need to be high precision are done with the GMP library. 



