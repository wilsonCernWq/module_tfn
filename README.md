# module_tfn

A standalone 2D transfer function widget implementation for OSPRay

### 2D Gradient Transfer Function

2D Transfer functions map the scalar values of volumes and the gradient magnitudes
of the scalar values to color and opacity.
To create a new transfer function of this type use

``` {.cpp}
OSPTransferFunction ospNewTransferFunction("piecewise_linear_2d");
```

This transfer function follows OSPRay's API for transfer function, which means
the created handle can be assigned to a volume as parameter "`transferFunction`"
using `ospSetObject`.

This new transfer function type can map 1D or 2D array to color and opacity, therefore
it requires more parameters. Currently pre-integration is not implemented for this
transfer function type.

| Type       | Name          | Description                                     |
|:-----------|:--------------|:------------------------------------------------|
| vec3f\[ \] | colors        | [data](#data) array of RGB colors               |
| float\[ \] | opacities     | [data](#data) array of opacities                |
| int        | colorWidth    | width (scalar axis) of colors                   |
| int        | colorHeight   | height (gradient axis) of colors                | 
| int        | opacityWidth  | width (scalar axis) of opacities                |
| int        | opacityHeight | height (gradient axis) of opacities             | 
| vec2f      | valueRange    | domain (scalar range) this function maps from   |
| vec2f      | gradRange     | domain (gradient range) this function maps from |

: Parameters accepted by the 2D gradient linear transfer function.
