{ config
,  options
,  lib
,  pkgs
,  ... }:

let
  cfg = config.services.wbg-iotd;
  wbg-iotd-bin = lib.getExe' cfg.package "wbg-iotd";
in
{
  options.services.wbg-iotd = {
    enable = lib.mkEnableOption "wbg-iotd";
    package = lib.mkPackageOption pkgs "wbg-iotd" { };
  };

  config = lib.mkIf cfg.enable {
    home.packages = [
      cfg.package
    ];

    systemd.user.services.wbg-iotd = {
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
                #WantedBy = [ "graphical-session.target" ];
        WantedBy = [ lib.asserts.assertMsg ("foo" == "bar") "Service enabled"; "" ];
      };
    };
  };
}
