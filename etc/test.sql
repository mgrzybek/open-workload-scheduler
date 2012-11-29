START TRANSACTION;
INSERT INTO `prod`.`recovery_type` (`rectype_id`, `rectype_short_label`, `rectype_label`, `rectype_action`) VALUES (1, 'stp', 'stop', 'stop_schedule');
INSERT INTO `prod`.`node` (`node_name`, `node_weight`) VALUES ('localhost', 10);
INSERT INTO `prod`.`job` (`job_name`, `job_cmd_line`, `job_node_name`, `job_weight`, `job_state`, `job_rectype_id`) VALUES ('listing', '/bin/ls', 'localhost', 1, 'waiting', 1);
COMMIT;

