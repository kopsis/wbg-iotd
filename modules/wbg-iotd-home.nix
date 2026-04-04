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

  config.systemd.user.services = lib.mkIf cfg.enable {
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
}
