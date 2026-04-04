{ config
,  options
,  lib
,  pkgs
,  ... }:

let
  cfg = config.services.wbg-iotd;
  wbg-iotd-bin = lib.getExe' cfg.package "wbg-iotd";
  eval = f: if lib.isFunction f then eval (f null) else f;
  tmpRule = type: name: mode: user: group: age: {
    "${name}"."${type}" = lib.filterAttrs (k: v: v != null) { inherit mode user group age; };
  };
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
        ConditionEnvironment = "WAYLAND_DISPLAY";
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

    systemd.user.tmpfiles.rules = [
      "d %h/.local/share/wallpaper - - - 30d"
    ];
  };
}
