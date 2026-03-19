
/* using System;

class Program
{
    static void Main()
    {
        string[] input = Console.ReadLine().Split();
        int a = int.Parse(input[0]);
        int b = int.Parse(input[1]);
        int c = int.Parse(input[2]);

        if (Solve(a, b, c, out int x, out int y))
        {
            Console.WriteLine($"{x} {y}");
        }
        else
        {
            Console.WriteLine("Impossible");
        }
    }

    static bool Solve(int a, int b, int c, out int x, out int y)
    {
        x = 0; y = 0;

        int gcd = NOD(a, b, out x, out y);

        // ЯВНАЯ проверка делимости
        if (gcd == 0 || c % gcd != 0)
        {
            return false;
        }

        // Масштабируем под c
        int multiplier = c / gcd;
        x *= multiplier;
        y *= multiplier;

        // Минимальное неотрицательное x
        int step_x = b / gcd;  // t = b/gcd
        if (step_x != 0 && x < 0)
        {
            int k = (int)Math.Ceiling((double)-x / step_x);
            x += k * step_x;
            y -= k * (a / gcd);
        }

        return true;
    }

    static int NOD(int a, int b, out int x, out int y)
    {
        x = 0; y = 0;

        if (a == 0)
        {
            y = 1; return b;
        }
        if (b == 0)
        {
            x = 1; return a;
        }

        int x1, y1;
        int gcd = NOD(b, a % b, out x1, out y1);

        x = y1;
        y = x1 - (a / b) * y1;
        return gcd;
    }
}
*/
using System;

class Program
{
    static void Main()
    {
        string[] input = Console.ReadLine().Split();
        long a = long.Parse(input[0]);
        long b = long.Parse(input[1]);
        long c = long.Parse(input[2]);

        // Find GCD and coefficients using Extended Euclidean Algorithm
        var result = ExtendedGcd(a, b);
        long gcd = result.Item1;
        long x0 = result.Item2;
        long y0 = result.Item3;

        // Check if solution exists
        if (c % gcd != 0)
        {
            Console.WriteLine("Impossible");
            return;
        }

        // Scale the solution
        long multiplier = c / gcd;
        x0 *= multiplier;
        y0 *= multiplier;

        // General solution: x = x0 + (b/gcd)*t, y = y0 - (a/gcd)*t
        long b_div = b / gcd;
        long a_div = a / gcd;

        // Find t such that x is the smallest non-negative value
        // x = x0 + b_div * t >= 0
        // t >= -x0 / b_div
        long t;
        if (b_div > 0)
        {
            t = (long)Math.Ceiling((double)(-x0) / b_div);
        }
        else
        {
            t = (long)Math.Floor((double)(-x0) / b_div);
        }

        long x = x0 + b_div * t;
        long y = y0 - a_div * t;

        Console.WriteLine($"{x} {y}");
    }

    // Extended Euclidean Algorithm
    // Returns (gcd, x, y) such that a*x + b*y = gcd
    static Tuple<long, long, long> ExtendedGcd(long a, long b)
    {
        if (b == 0)
            return Tuple.Create(a, 1L, 0L);

        var result = ExtendedGcd(b, a % b);
        long gcd = result.Item1;
        long x1 = result.Item2;
        long y1 = result.Item3;

        long x = y1;
        long y = x1 - (a / b) * y1;

        return Tuple.Create(gcd, x, y);
    }
}