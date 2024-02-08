def test_is_In_Corner():
    assert is_In_Corner(0, 0, [(0, 0), (0, 1), (1, 1), (1, 0)]) is False
    assert is_In_Corner(0.5, 0.5, [(0, 0), (0, 1), (1, 1), (1, 0)]) is True
    assert is_In_Corner(0.5, 0.5, [(0, 0), (1, 0), (1, 1), (0, 1)]) is True
    assert is_In_Corner(0.5, 0.5, [(0, 0), (1, 0), (1, 1), (0, 1),
                                   (0.5, 0.5)]) is False
    assert is_In_Corner(0, 0, []) is False
    assert is_In_Corner(10, 10, [(0, 0), (10, 0), (10, 10)]) is False
    assert is_In_Corner(0, 10, [(0, 0), (10, 0), (10, 10)]) is True
    assert is_In_Corner(10, 0, [(0, 0), (10, 0), (10, 10)]) is True
    assert is_In_Corner(5, 5, [(0, 0), (10, 0), (10, 10), (0, 10)]) is False
    assert is_In_Corner(5, 0, [(0, 0), (10, 0), (10, 10), (0, 10)]) is True
    assert is_In_Corner(0, 5, [(0, 0), (10, 0), (10, 10), (0, 10)]) is True
    assert is_In_Corner(5, 10, [(0, 0), (10, 0), (10, 10), (0, 10)]) is True
