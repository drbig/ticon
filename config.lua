os = require("os")

icon = {
  kind = "stock",
  path = "gtk-home",
  title = "My Lua Tray Icon",
}

menu = {
  {"XTerm", function ()
    now = os.date("*t")
    if (now.hour > 6) and (now.hour < 22) then
      print("day xterm")
      os.execute("xterm +rv &")
    else
      print("night xterm")
      os.execute("xterm -rv &")
    end
  end},
  {"One", function ()
    print("one")
  end},
  {"Two", function ()
    print("two")
  end},
  {"Three", function ()
    print("three")
  end},
  {"This is a test", function ()
    print("test")
  end},
}

print("Config loaded.")
