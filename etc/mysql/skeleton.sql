SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

-- We skip the warnings
SET sql_notes = 0;

-- -----------------------------------------------------
-- Table `node`
-- -----------------------------------------------------

CREATE TABLE `node` (
	`node_name` VARCHAR(45) NOT NULL ,
	`node_weight` INT(11) NOT NULL DEFAULT '0' ,
	PRIMARY KEY (`node_name`)
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

-- -----------------------------------------------------
-- Table `recovery_type`
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `recovery_type` (
	`rectype_id` INT(11) NOT NULL AUTO_INCREMENT ,
	`rectype_short_label` VARCHAR(45) NOT NULL ,
	`rectype_label` VARCHAR(45) NOT NULL ,
	`rectype_action` ENUM('restart','stop_schedule') NOT NULL ,
	PRIMARY KEY (`rectype_id`)
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1
COMMENT = '\'Deals with what to do after the end of a failed job' ;

-- -----------------------------------------------------
-- Table `macro_job`
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `macro_job` (
	`macro_id` INT(11) NOT NULL AUTO_INCREMENT ,
	`macro_name` VARCHAR(45) NOT NULL ,
	PRIMARY KEY (`macro_id`) ,
	INDEX `fk_macro_id` (`macro_id` ASC)
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1,
COMMENT = '\'This is an abstract object' ;

-- -----------------------------------------------------
-- Table `job`
-- job_macro_job_id should be NOT NULL
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `job` (
	`job_name` VARCHAR(45) NOT NULL ,
	`job_cmd_line` VARCHAR(45) NOT NULL ,
	`job_node_name` VARCHAR(45) NOT NULL ,
	`job_weight` INT(11) NOT NULL DEFAULT '1' ,
	`job_start_time` TIME DEFAULT NULL ,
	`job_stop_time` TIME DEFAULT NULL ,
	`job_state` ENUM('waiting','running','succeded','failed') NULL DEFAULT 'waiting' ,
	`job_rectype_id` INT(11) NULL DEFAULT NULL ,
	`job_macro_job_id` INT(11) ,
	PRIMARY KEY (`job_name`) ,
	INDEX `fk_job_node` (`job_node_name` ASC) ,
	INDEX `fk_rectype_id` (`job_rectype_id` ASC) ,
	INDEX `fk_job_macro_job1` (`job_macro_job_id` ASC) ,
	CONSTRAINT `fk_job_node`
		FOREIGN KEY (`job_node_name` )
		REFERENCES `node` (`node_name` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION,
	CONSTRAINT `fk_rectype_id`
		FOREIGN KEY (`job_rectype_id` )
		REFERENCES `recovery_type` (`rectype_id` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION,
	CONSTRAINT `fk_job_macro_job1`
		FOREIGN KEY (`job_macro_job_id` )
		REFERENCES `macro_job` (`macro_id` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
)
ENGINE = InnoDB
AUTO_INCREMENT = 5
DEFAULT CHARACTER SET = latin1;

-- -----------------------------------------------------
-- Table `resource`
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `resource` (
	`resource_name` VARCHAR(45) NOT NULL ,
	`resource_node_name` VARCHAR(11) NOT NULL ,
	`resource_current_value` INT(11) NOT NULL ,
	`resource_initial_value` INT(11) NOT NULL ,
	INDEX `fk_resource_node` (`resource_node_name` ASC) ,
	PRIMARY KEY (`resource_name`, `resource_node_name`) ,
	CONSTRAINT `fk_resource_node`
		FOREIGN KEY (`resource_node_name` )
		REFERENCES `node` (`node_name` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

-- -----------------------------------------------------
-- Table `time_constraint`
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `time_constraint` (
	`time_c_type` ENUM('at','before','after') NOT NULL ,
	`time_c_value` TIME NOT NULL ,
	`time_c_job_name` VARCHAR(45) NOT NULL ,
	PRIMARY KEY (`time_c_type`, `time_c_job_name`) ,
	INDEX `fk_time_constraint_job1` (`time_c_job_name` ASC) ,
	CONSTRAINT `fk_time_constraint_job1`
		FOREIGN KEY (`time_c_job_name` )
		REFERENCES `job` (`job_name` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

-- -----------------------------------------------------
-- Table `jobs_link`
-- -----------------------------------------------------

CREATE TABLE IF NOT EXISTS `jobs_link` (
	`job_name_prv` VARCHAR(45) NOT NULL ,
	`job_name_nxt` VARCHAR(45) NOT NULL ,
	PRIMARY KEY (`job_name_prv`, `job_name_nxt`) ,
	INDEX `fk_job_has_job_job2` (`job_name_nxt` ASC) ,
	INDEX `fk_job_has_job_job1` (`job_name_prv` ASC) ,
	CONSTRAINT `fk_job_has_job_job1`
		FOREIGN KEY (`job_name_prv` )
		REFERENCES `job` (`job_name` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION,
	CONSTRAINT `fk_job_has_job_job2`
		FOREIGN KEY (`job_name_nxt` )
		REFERENCES `job` (`job_name` )
		ON DELETE NO ACTION
		ON UPDATE NO ACTION
)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

-- -----------------------------------------------------
-- Placeholder table for view `get_ready_job`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `get_ready_job` (`job_name` VARCHAR(45), `job_cmd_line` INT, `job_node_name` INT, `job_weight` INT, `job_state` INT, `job_rectype_id` INT);

-- -----------------------------------------------------
-- Placeholder table for view `get_ready_time`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `get_ready_time` (`job_name` VARCHAR(45));

-- -----------------------------------------------------
-- Placeholder table for view `get_ready_links`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `get_ready_links` (`job_name` VARCHAR(45));

-- -----------------------------------------------------
-- Placeholder table for view `get_ready_linkless`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `get_ready_linkless` (`job_name` VARCHAR(45));

-- -----------------------------------------------------
-- Placeholder table for view `get_available_resource`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `get_available_resource` (`resource_id` INT, `resource_name` INT, `resource_node_name` INT, `resource_value` INT);

-- -----------------------------------------------------
-- View `get_ready_job`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `get_ready_job` ;
DROP TABLE IF EXISTS `get_ready_job`;

CREATE OR REPLACE ALGORITHM=UNDEFINED
	VIEW `get_ready_job` AS select `j`.`job_name` AS `job_name`,`j`.`job_cmd_line` AS `job_cmd_line`,`j`.`job_node_name` AS `job_node_name`,`j`.`job_weight` AS `job_weight`,`j`.`job_state` AS `job_state`,`j`.`job_rectype_id` AS `job_rectype_id` from `job` `j`
	where (
			(
			 `j`.`job_name` in (select `get_ready_links`.`job_name` AS `job_name` from `get_ready_links`)
			 xor
			 `j`.`job_name` in (select `get_ready_linkless`.`job_name` AS `job_name` from `get_ready_linkless`)
			)
			and `j`.`job_name` in (select `j`.`job_name` AS `job_name` from `get_ready_time`)
	      )
	;

-- -----------------------------------------------------
-- View `get_ready_time`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `get_ready_time` ;
DROP TABLE IF EXISTS `get_ready_time`;

CREATE OR REPLACE ALGORITHM=UNDEFINED
	VIEW `get_ready_time` AS select job_name from job j
	where
j.job_name not in (select time_c_job_name from time_constraint)
	xor (
			j.job_name in (select time_c_job_name from time_constraint where (time_c_type = 'at' and time_format(time_c_value,'%H %i') = time_format(now(),'%H %i')))
			or j.job_name in (select time_c_job_name from time_constraint where (time_c_type = 'before' and time_format(time_c_value,'%H %i') >= time_format(now(),'%H %i')))
			or j.job_name in (select time_c_job_name from time_constraint where (time_c_type = 'after' and time_format(time_c_value,'%H %i') <= time_format(now(),'%H %i')))
	    )
	;

-- -----------------------------------------------------
-- View `get_ready_links`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `get_ready_links` ;
DROP TABLE IF EXISTS `get_ready_links`;

CREATE OR REPLACE ALGORITHM=UNDEFINED
	VIEW `get_ready_links` AS select distinct `jl`.`job_name_nxt` AS `job_name` from (`jobs_link` `jl` join `job` `j`) where (`jl`.`job_name_prv` = `j`.`job_name`) and (`j`.`job_state` = 'succeded');

-- -----------------------------------------------------
-- View `get_ready_linkless`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `get_ready_linkless` ;
DROP TABLE IF EXISTS `get_ready_linkless`;

CREATE OR REPLACE ALGORITHM=UNDEFINED
	VIEW `get_ready_linkless` AS SELECT `job_name` FROM `job` WHERE `job_state` = 'waiting' AND `job_name` NOT IN ( SELECT `job_name_nxt` FROM `jobs_link` );

-- -----------------------------------------------------
-- View `get_available_resource`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `get_available_resource` ;
DROP TABLE IF EXISTS `get_available_resource`;

CREATE OR REPLACE ALGORITHM=UNDEFINED
	VIEW `get_available_resource` AS select `resource`.`resource_name` AS `resource_name`,`resource`.`resource_node_name` AS `resource_node_name`,`resource`.`resource_current_value` AS `resource_current_value` from `resource` where (`resource`.`resource_current_value` > 0);

-- We re activate the warnings
SET sql_notes = 1;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

