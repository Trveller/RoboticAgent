/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package iamr.lab1;

import java.util.Random;

/**
 *
 * @author vedran
 */
public class HelperFunctions {
    
    /**
     * Returns a pseudo-random number between min and max, inclusive.
     * The difference between min and max can be at most
     * <code>Integer.MAX_VALUE - 1</code>.
     *
     * @param min Minimum value
     * @param max Maximum value.  Must be greater than min.
     * @return Integer between min and max, inclusive.
     * @see java.util.Random#nextInt(int)
     */
    public static int randInt(int min, int max) {

        // NOTE: Usually this should be a field rather than a method
        // variable so that it is not re-seeded every call.
        Random rand = new Random();

        // nextInt is normally exclusive of the top value,
        // so add 1 to make it inclusive
        int randomNum = rand.nextInt((max - min) + 1) + min;

        return randomNum;
    }
    
    /**
     * @brief Returns a pseudo-random boolean
     */
    public static boolean randBool() {
        Random rand = new Random();
        return rand.nextBoolean();
    }
    
    /**
     * @brief prints message to standard output
     */
    public static void print_to_output(String message) {
        System.out.println( message );
    }
    
}
