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
    stencilShift,
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


@pytest.mark.parametrize(
    "points, shift_size, shift_dim, expected",
    [
        ([Point([1, 2, 3])], 2, 0, [Point([3, 2, 3])]),
        ([Point([1, 2, 3])], -2, 1, [Point([1, 0, 3])]),
        ([Point([1, 2, 3]), Point([-1, 0, 1])], 4, 2,
            [Point([1, 2, 7]), Point([-1, 0, 5])]),
    ],
)
def test_stencil_shift(points, shift_size, shift_dim, expected):
    result = stencilShift(points, shift_size, shift_dim)

    assert result == expected
    assert all(
        result_point is not original_point
        for result_point, original_point in zip(result, points)
    )


@pytest.mark.parametrize("shift_dim", [-1, 3])
def test_stencil_shift_rejects_invalid_dimension(shift_dim):
    with pytest.raises(OpsError, match=r"shift_dim must be in \[0, 2\]"):
        stencilShift([Point([0, 0, 0])], 1, shift_dim)
