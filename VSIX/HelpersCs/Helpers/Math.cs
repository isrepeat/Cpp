﻿namespace Helpers {
    public static class Math {
        public static double Clamp(double value, double min, double max) {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }
    }
}