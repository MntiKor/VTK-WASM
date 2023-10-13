# Map Six Cube Faces to an Equirectangular File

How to map 6 cube (cubemap, skybox) faces into an equirectangular (cylindrical projection, skysphere) map.

We will use [cube2sphere](https://pypi.org/project/cube2sphere/) installing it into a virtual environment.

First download blender, then create a virtual environment and install cube2sphere

``` bash
conda create -n xyene
conda activate xyene
conda install pip
pip install cube2sphere
```

To use it, go to the folder where the cube files are and open a console then:

```
conda activate xyene
```

We initially need to run this command depending on which folder we are in:

- skybox0

    ``` bash
    cube2sphere back.jpg front.jpg right.jpg left.jpg top.jpg bottom.jpg -r 4096 2048 -fPNG -t12 -R 0 0 90
    ```

- skybox1

    ``` bash
    cube2sphere skybox-nz.jpg skybox-pz.jpg skybox-px.jpg skybox-nx.jpg skybox-py.jpg skybox-ny.jpg -r 2048 1024 -fPNG -t12 -R 0 0 90
    ```

- skybox2

    ``` bash
    cube2sphere negz.jpg posz.jpg posx.jpg negx.jpg posy.jpg negy.jpg -r 2048 1024 -fPNG -t12 -R 0 0 90
    ```

Then load the resultant file `out0001.png` into an image editor, mirror the image, and save it as `skybox.jpg` with JPEG quality 100%. Some detail will be lost.
