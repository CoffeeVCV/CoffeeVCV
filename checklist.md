# Checklist

1. Write a short design brief which should include :
   * the perpose of the module
     * This will be useful for...
   * the compoents you expect to use
   * the basic operation
   * sketch on paper
2. Instantiate the new module in the code base
   * Copy one the base svg
   * User the helper
   * Update plugin.ccp and pluggin.hpp
3. Modify cpp
   * edit ENUMS
   * basic configs
   * rought widget componnent placements
     * left, middle, right
     * 10mm vertical separation sia good start
   * outputs at the bottom
4. develop and test
   * test
   * test
   * test
   * commit in decrete chunks
   * oh, I have a great idea
     * check design brief
     * start a new brief if it's a new module, walk away
   * repeat
5. Publish
   * clean code
   * lint code
   * update documentation
     * Include images
   * Update changelog
   * Update version in manifest
   * Push upstream
   * open ticket for lib update