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
    alert("one")
  end},
  {"Two", function ()
    alert("two")
  end},
  {"Three", function ()
    alert("three")
  end},
  {"This is a test", function ()
    alert("test")
  end},
  {"Quit", function ()
    os.exit(0);
  end},
}

print("Config loaded.")
