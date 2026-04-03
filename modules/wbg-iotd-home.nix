{
  config,
  options,
  lib,
  pkgs,
  ...
}:
with lib;
let
  cfg = config.wbg-iotd;
  wbg-iotd-bin = lib.getExe config.wbg-iotd.package;
in
  options.wbg-iotd = {
  };

  home.packages = [
    config.wbg-iotd.package; 
  ];

  systemd.user.services = {
    wbg-iotd = {
      Unit = {
        Description = "Wallpaper image-of-the-day manager.";
        PartOf = [ "graphical-session.target" ];
        After = [ "graphical-session.target" ];
      };
      Service = {
        Type = "exec";
        ExecStart = "${wbg-iotd-bin}";
        Restart = "always";
        RestartSec = 10;
      };
      Install = {
        WantedBy = [ "graphical-session.target" ];
      };
    };
  };
};
