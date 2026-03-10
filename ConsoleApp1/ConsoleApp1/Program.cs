using System;

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

        if (gcd != 0 && c % gcd == 0)
        {
            x *= c / gcd;
            y *= c / gcd;

            int b_gcd = b / gcd;
            int k = (int)Math.Ceiling((double)-x / b_gcd);
            x += k * b_gcd;
            y -= k * (a / gcd);

            return true;
        }
        return false;
    }

    static int NOD(int a, int b, out int x, out int y)
    {
        if (b == 0)
        {
            x = 1; y = 0;
            return a;
        }

        int x1, y1;
        int gcd = NOD(b, a % b, out x1, out y1);

        x = y1;
        y = x1 - (a / b) * y1;
        return gcd;
    }
}
