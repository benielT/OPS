"""Unit tests for the stencil geometry helpers in ops.py (pure Python, no libclang)."""
import pytest
 
from ops import (
    OpsError,
    Point,
    arrayToPoints,
    computeWidenPoints,
    genRowDiscriptors,
    getMinPoint,
    getStencilSize,
    stencilPointsSort,
)
 
 
def test_point_arithmetic():
    a = Point([1, 2, 3])
    b = Point([1, 1, 1])
    assert a + b == Point([2, 3, 4])
    assert a - b == Point([0, 1, 2])
    assert -a == Point([-1, -2, -3])
 
def test_stencil_size_and_min_point_2d_star():
    pts = arrayToPoints(5, 2, [0, 0, 1, 0, -1, 0, 0, 1, 0, -1])
    assert getStencilSize(pts) == Point([3, 3, 1])
    assert getMinPoint(pts) == Point([-1, -1, 0])
    
@pytest.mark.parametrize("vector_factor, expected_width", [(1, 3), (2, 3), (8, 3)])
def test_widen_points_x_extent(vector_factor, expected_width):
    pts = stencilPointsSort(5, 2, [0, 0, 1, 0, -1, 0, 0, 1, 0, -1])
    widen, mapping = computeWidenPoints(genRowDiscriptors(pts), vector_factor)
    xs = [p.x for p in widen]
    assert max(xs) - min(xs) + 1 == expected_width
    assert min(xs) == 0  # translated to origin
    assert set(mapping) == set(pts)  # every original point is mapped