/*
 * Temperv14 Linux Kernel Module Driver by Justin Yang
 * 
 * Temper driver for linux. The driver will work with some
 * TEMPer usb devices from RDing (www.PCsensor.com).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 * THIS SOFTWARE IS PROVIDED BY Justin S. Yang ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Justin Yang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/delay.h>

#include <asm/uaccess.h>

#define T_CTRL_BUFFER_SIZE 8
#define T_CTRL_REQUEST 0x09
#define T_CTRL_REQUEST_TYPE 0x21
#define T_CTRL_VALUE 0x0200
#define T_CTRL_INDEX 0x01 
#define T_VENDOR_ID 0x0c45
#define T_PRODUCT_ID 0x7401

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))

#define T_MINOR_BASE 0
#define MAX_PKT_SIZE 8

static struct usb_driver t_driver;
static struct usb_device *t_device;
//static unsigned char bulk_buf[MAX_PKT_SIZE];

struct usb_t {
	struct usb_device *udev;
	struct usb_interface *interface;
	unsigned char minor;
	char serial_number[8];
	
	unsigned char *int_in_buffer;
	struct usb_endpoint_descriptor *int_in_endpoint;
	struct urb *int_in_urb;
	int int_in_running;
	
	unsigned char *ctrl_buffer;
	struct urb *ctrl_urb;
	struct usb_ctrlrequest *ctrl_dr;
};

static struct usb_device_id t_table[] = {
	{ USB_DEVICE(T_VENDOR_ID, T_PRODUCT_ID) },
	{ }
};

//Callbacks
static void t_int_in_callback(struct urb *urb) {
	
	struct usb_t *dev = urb->context;
	printk("CALLBACK!\n");
	printk("Buffer(callback): ");
	int i;
	for(i=0; i<4; i++)
	printk("%02x",dev->int_in_buffer[i]);
	printk("\n");
	
	printk("STATUS of URB: %d\n",urb->status);
}

static void t_ctrl_callback(struct urb *urb)
{
	//struct usb_ml *dev = urb->context;
}

static inline void t_delete(struct usb_t *dev)
{
//	t_abort_transfers(dev);

	/* Free data structures. */
	if (dev->int_in_urb)
		usb_free_urb(dev->int_in_urb);
	if (dev->ctrl_urb)
		usb_free_urb(dev->ctrl_urb);

	kfree(dev->int_in_buffer);
	kfree(dev->ctrl_buffer);
	kfree(dev->ctrl_dr);
	kfree(dev);
}

static int t_open(struct inode *inode, struct file *file) {

	struct usb_t *dev = NULL;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	printk("Open device\n");
	subminor = iminor(inode);


	interface = usb_find_interface(&t_driver, subminor);
	if (! interface) {
		printk("can't find device for minor %d", subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (! dev) {
		retval = -ENODEV;
		goto exit;
	}
	/* Initialize interrupt URB. */
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
			usb_rcvintpipe(dev->udev,
				       dev->int_in_endpoint->bEndpointAddress),
			dev->int_in_buffer,
			le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize),
			t_int_in_callback,
			dev,
			dev->int_in_endpoint->bInterval);

	dev->int_in_running = 1;
	//mb();

	/* Save our object in the file's private structure. */
	file->private_data = dev;


exit:
	return retval;
}

static ssize_t t_read(struct file *file, char __user *buf, size_t count, loff_t *off) {

	printk("Read from device\n");
	int retval;
	//int read_cnt;

	struct usb_t *dev = NULL;
	dev = file->private_data;

	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);	
	msleep(100);
	int i;
	if (copy_to_user(buf, dev->int_in_buffer, sizeof(dev->int_in_buffer)))
			return -EFAULT;

	for(i=0; i<4; i++)
		printk("%02x",dev->int_in_buffer[i]);
	printk("\n");
	
	if (retval) {
		printk("submitting int urb failed (%d)", retval);
		dev->int_in_running = 0;
	}
	return MIN(count, sizeof(dev->int_in_buffer));
}

static ssize_t t_write(struct file *file, const char __user *user_buf, size_t cnt, loff_t *ppos) {

	printk("Write to device\n");
	
	//int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);
	struct usb_t *dev;
	int retval = 0;

	dev = file->private_data;
	u8 buf[8];
	copy_from_user(buf,user_buf,cnt);
//Easier way is to use usb_control_msg	
/*	retval = usb_control_msg(dev->udev,
			usb_sndctrlpipe(dev->udev, 0),
			T_CTRL_REQUEST,
			T_CTRL_REQUEST_TYPE,
			T_CTRL_VALUE,
			T_CTRL_INDEX,
			buf,
			sizeof(buf),
			HZ*5);
*/
	memcpy(dev->ctrl_buffer,buf,T_CTRL_BUFFER_SIZE);
	retval = usb_submit_urb(dev->ctrl_urb,GFP_KERNEL);
	if (retval < 0) {
		printk("usb_control_msg failed (%d)", retval);
	}
	
	return MIN(cnt,sizeof(dev->ctrl_buffer));
}

static struct file_operations t_fops = {
	.owner = THIS_MODULE,
	.write = t_write,
	.open  = t_open,
	.read  = t_read,
//	.release = t_release
};

static struct usb_class_driver t_class = {
	.name = "temp%d",
	.fops = &t_fops,
	.minor_base = T_MINOR_BASE
};


static int t_probe(struct usb_interface *interface, const struct usb_device_id *id) {
	t_device = interface_to_usbdev(interface);
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_t *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i, int_end_size;
	int retval = -ENODEV;
	printk("Probe TEMPer\n");

	dev = kzalloc(sizeof(struct usb_t), GFP_KERNEL);
	dev->udev = udev;
	dev->interface = interface;
	iface_desc = interface->cur_altsetting;
	
	printk("# of endpoints: %d\n",iface_desc->desc.bNumEndpoints);	
	for(i=0; i< iface_desc->desc.bNumEndpoints;i++) {
		endpoint = &iface_desc->endpoint[i].desc;
		
		if(endpoint->bEndpointAddress == 0x81)
			return -1;
		if(((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN) && 
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT)) 
			dev->int_in_endpoint = endpoint;
			printk("ENDPOINT: %08x\n", endpoint->bEndpointAddress);
	}

		
	int_end_size = le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize);
	dev->int_in_buffer = kmalloc(int_end_size, GFP_KERNEL);
	
	dev->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
/*
        usb_fill_int_urb(dev->int_in_urb, dev->udev,
                        usb_rcvintpipe(dev->udev,
                                       dev->int_in_endpoint->bEndpointAddress),
                        dev->int_in_buffer,
                        le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize),
                        t_int_in_callback,
                        dev,
                        dev->int_in_endpoint->bInterval);
*/
	dev->ctrl_urb = usb_alloc_urb(0,GFP_KERNEL);
	dev->ctrl_buffer = kmalloc(8,GFP_KERNEL);
        dev->ctrl_dr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);

	dev->ctrl_dr->bRequestType = T_CTRL_REQUEST_TYPE;
	dev->ctrl_dr->bRequest = T_CTRL_REQUEST;
	dev->ctrl_dr->wValue = cpu_to_le16(T_CTRL_VALUE);
	dev->ctrl_dr->wIndex = cpu_to_le16(T_CTRL_INDEX);
	dev->ctrl_dr->wLength = cpu_to_le16(T_CTRL_BUFFER_SIZE);

	usb_fill_control_urb(dev->ctrl_urb, dev->udev,
			usb_sndctrlpipe(dev->udev, 0),
			(unsigned char *)dev->ctrl_dr,
			dev->ctrl_buffer,
			T_CTRL_BUFFER_SIZE,
			t_ctrl_callback,
			dev);

	/* Retrieve a serial. */
	if (! usb_string(udev, udev->descriptor.iSerialNumber,
			 dev->serial_number, sizeof(dev->serial_number))) {
		printk("could not retrieve serial number");
		goto error;
	}

	/* Save our data pointer in this interface device. */
	usb_set_intfdata(interface, dev);

	/* We can register the device now, as it is ready. */
	retval = usb_register_dev(interface, &t_class);
	if (retval) {
		printk("not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	dev->minor = interface->minor;

//exit:
	printk("PROBE SUCCESS\n");
	return retval;

error:
	t_delete(dev);
	return retval;
}

static void t_disconnect(struct usb_interface *interface) {
	
	struct usb_t *dev;
	int minor;
	
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	minor = dev->minor;
	
	usb_deregister_dev(interface, &t_class);
	
	printk("TEMPer /dev/temp%d now disconnected\n",minor - T_MINOR_BASE);
}

static struct usb_driver t_driver = {
	.name = "TEMPer",
	.id_table = t_table,
	.probe = t_probe,
	.disconnect = t_disconnect

};

static int __init usb_t_init(void) {

	int result;
	printk("Register driver\n");
	result = usb_register(&t_driver);
	if(result) {
		printk("Registering driver failed!\n");
	} else {
		printk("Driver registered successfully!\n");
	}
	return result;
}

static void __exit usb_t_exit(void) {

	usb_deregister(&t_driver);
	printk("Module deregistered!\n");
}

module_init(usb_t_init);
module_exit(usb_t_exit);
MODULE_LICENSE("GPL");
