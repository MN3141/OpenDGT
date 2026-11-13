import tkinter as tk
import serial

ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)


def help_btn_cmd():

    global _help_win_instances
    if _help_win_instances == 0:
        _help_win_instances = _help_win_instances + 1
        win = tk.Toplevel()
        win.title("OpenDGTCom Help")


def send_str():

    global txt_field_handler
    global ser
    dgt3000_max_symbols = 11

    rx_str = txt_field_handler.get("1.0", "end-1c")
    str_len = len(rx_str)

    win_handler = tk.Toplevel()
    win_handler.geometry("680x480")

    if str_len > dgt3000_max_symbols:
        win_title = "TX Error!"
        msg = f"Please use less than {dgt3000_max_symbols} symbols! Currently you are trying to send {str_len} symbols."
    else:
        win_title = "RX complete!"
        msg = f"{rx_str} has been sent!"

        ser.write(rx_str.encode("utf-8"))

    win_handler.title(win_title)
    label_handler = tk.Label(win_handler, text=msg)
    label_handler.pack()


_help_win_instances = 0

root = tk.Tk()
root.title("OpenDGTCom")

frm_handle = tk.Frame(root)
frm_handle.grid()

help_btn_handle = tk.Button(frm_handle, text="Help", command=help_btn_cmd)
help_btn_handle.grid(row=0, column=2)
send_btn_handle = tk.Button(frm_handle, text="Send", command=send_str)

send_btn_handle.grid(row=1, column=1)
txt_field_handler = tk.Text(frm_handle)
txt_field_handler.grid(row=1, column=0)


def main():

    root.mainloop()


if __name__ == "__main__":
    main()
