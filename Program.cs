
using System;

class Program
{
    static void Main()
    {
        string[] input = Console.ReadLine().Split();
        long a = long.Parse(input[0]);
        long b = long.Parse(input[1]);
        long c = long.Parse(input[2]);

        var result = ExtendedGcd(a, b);
        long gcd = result.Item1;
        long x0 = result.Item2;
        long y0 = result.Item3;

        if (c % gcd != 0)
        {
            Console.WriteLine("Impossible");
            return;
        }

        long multiplier = c / gcd;
        x0 *= multiplier;
        y0 *= multiplier;

        long b_div = b / gcd;
        long a_div = a / gcd;


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