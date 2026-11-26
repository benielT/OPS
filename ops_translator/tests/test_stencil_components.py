"""Unit tests for the stencil geometry helpers in ops.py (pure Python, no libclang)."""
import pytest
 
from ops import (
    OpsError,
    Point,
    Stencil,
    StencilVisualizer,
    arrayToPoints,
    computeWidenPoints,
    genRowDiscriptors,
    getMinPoint,
    getStencilSize,
    stencilPointsSort,
    stencilShift,
    yVectStencilGeneration,
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


def test_stencil_visualizer_to_string_2d_labels_axes():
    points = [Point([-12, 2, 0]), Point([3, -10, 0])]

    output = StencilVisualizer.to_string_2d(points)
    lines = output.splitlines()
    x_values = lines[1].split("|", 1)[1].split()
    y_values = [
        line.split("|", 1)[0].strip() for line in lines[2:-1]
    ]

    assert lines[1].split("|", 1)[0].strip() == "y/x"
    assert x_values == [str(x) for x in range(-12, 4)]
    assert y_values == [str(y) for y in range(2, -11, -1)]
    
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


Y_VECT_STENCIL_CASES = [
    pytest.param(
        "2D second-order cross",
        2,
        [(0, 0), (1, 0), (-1, 0), (0, 1), (0, -1)],
        "cross_2d",
        id="2d-cross",
    ),
    pytest.param(
        "2D second-order rectangle",
        2,
        [(x, y) for y in (-1, 0, 1) for x in (-1, 0, 1)],
        "rectangle_2d",
        id="2d-rectangle",
    ),
    pytest.param(
        "2D second-order skew (cross plus principal diagonal)",
        2,
        [(0, 0), (1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (-1, -1)],
        "skew_2d",
        id="2d-skew",
    ),
    pytest.param(
        "3D second-order cross",
        3,
        [
            (0, 0, 0),
            (1, 0, 0),
            (-1, 0, 0),
            (0, 1, 0),
            (0, -1, 0),
            (0, 0, 1),
            (0, 0, -1),
        ],
        "cross_3d",
        id="3d-cross",
    ),
]


def _stencil_from_offsets(name, dim, offsets):
    points_3d = [tuple(point) + (0,) * (3 - len(point)) for point in offsets]
    minimum = [min(point[axis] for point in points_3d) for axis in range(3)]
    points = [
        Point([point[axis] - minimum[axis] for axis in range(3)])
        for point in points_3d
    ]
    d_m = Point(
        [abs(min(point[axis] for point in points_3d)) for axis in range(3)]
    )
    d_p = Point(
        [abs(max(point[axis] for point in points_3d)) for axis in range(3)]
    )
    return Stencil(
        0,
        dim,
        name,
        len(points),
        points,
        Point([-value for value in minimum]),
        getStencilSize(points),
        d_m,
        d_p,
    )


def _expected_y_vect_points(stencil_kind, y_vect_factor):
    if stencil_kind == "cross_2d":
        return {
            (x, y, 0)
            for x in range(3)
            for y in range(y_vect_factor + 2)
            if (x, y) != (0, 0)
        }

    if stencil_kind in ("rectangle_2d", "skew_2d"):
        return {
            (x, y, 0)
            for x in range(3)
            for y in range(y_vect_factor + 2)
        }

    if stencil_kind == "cross_3d":
        end_plane_rows = {2: range(1, 3), 4: range(1, 5), 8: range(1, 9)}
        mid_plane_start_y = {2: 0, 4:-2, 8:-6}
        points = {
            (x, y, 1) for x in range(3) for y in range(mid_plane_start_y[y_vect_factor], y_vect_factor + 2)
        }
        points.update(
            (1, y, z) for z in (0, 2) for y in end_plane_rows[y_vect_factor]
        )
        return points


@pytest.mark.parametrize("y_vect_factor", [2, 4, 8])
@pytest.mark.parametrize(
    "stencil_name,dim,offsets,stencil_kind", Y_VECT_STENCIL_CASES
)
def test_y_vect_stencil_generation(
    stencil_name, dim, offsets, stencil_kind, y_vect_factor, monkeypatch
):
    monkeypatch.setattr(
        StencilVisualizer, "save_as_image", lambda *args, **kwargs: None
    )
    stencil = _stencil_from_offsets(stencil_name, dim, offsets)
    expected_points = _expected_y_vect_points(stencil_kind, y_vect_factor)
    result = yVectStencilGeneration(stencil, y_vect_factor)
    actual_points = {(point.x, point.y, point.z) for point in result.points}

    print(
        f"Original input ({stencil_name}):\n"
        f"{StencilVisualizer.to_string(stencil.points, stencil.base_point)}"
    )
    expected_stencil_points = [
        Point(list(point)) for point in sorted(expected_points)
    ]
    print(
        f"Expected output (y vector factor {y_vect_factor}):\n"
        f"{StencilVisualizer.to_string(expected_stencil_points, result.base_point)}"
    )
    print(
        f"Given output (y vector factor {y_vect_factor}):\n"
        f"{StencilVisualizer.to_string(result.points, result.base_point)}"
    )

    assert actual_points == expected_points
