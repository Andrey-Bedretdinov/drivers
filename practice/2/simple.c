#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading module into the kernel
#include <linux/kernel.h>           
 
MODULE_LICENSE("GPL");              ///< The license type -- this affects function visibility
MODULE_AUTHOR("Alex Maximov");      ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux module");  ///< The description -- see modinfo
MODULE_VERSION("1.0");              

 
/** @brief The module initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that  the function is only used at initialization and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init simple_init(void){
   printk(KERN_INFO "Hello from the mai 307 team\n");
   return 0;
}
 
/** @brief The modul cleanup function
 */
static void __exit simple_exit(void){
   printk(KERN_INFO "Goodbye from the mai 307 team!\n");
}
 
module_init(simple_init);
module_exit(simple_exit);
