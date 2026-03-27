#!/usr/bin/env python3
"""Small sensor processing utility for rapid experiments on Raspberry Pi or laptop."""

from __future__ import annotations

from collections import deque
from statistics import median
from typing import Deque, Iterable


class MedianFilter:
    def __init__(self, size: int = 5) -> None:
        if size < 1:
            raise ValueError("size must be >= 1")
        self.size = size
        self.window: Deque[float] = deque(maxlen=size)

    def update(self, sample: float) -> float:
        self.window.append(sample)
        return median(self.window)


def exponential_smoothing(samples: Iterable[float], alpha: float = 0.25) -> list[float]:
    """Return smoothed values using first-order IIR filtering."""
    if not 0 < alpha <= 1:
        raise ValueError("alpha must be in (0, 1]")

    result: list[float] = []
    last = None
    for value in samples:
        if last is None:
            last = value
        else:
            last = alpha * value + (1 - alpha) * last
        result.append(last)
    return result


def demo() -> None:
    distance_samples = [62, 60, 61, 120, 59, 58, 57, 58]
    filt = MedianFilter(size=3)
    cleaned = [filt.update(v) for v in distance_samples]
    smooth = exponential_smoothing(cleaned, alpha=0.35)

    print("raw      :", distance_samples)
    print("median   :", [round(v, 2) for v in cleaned])
    print("smoothed :", [round(v, 2) for v in smooth])


if __name__ == "__main__":
    demo()
