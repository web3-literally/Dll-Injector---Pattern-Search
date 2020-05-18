/*
 Navicat Premium Data Transfer

 Source Server         : Localhost-MySql
 Source Server Type    : MySQL
 Source Server Version : 100130
 Source Host           : localhost:3306
 Source Schema         : dll_inject

 Target Server Type    : MySQL
 Target Server Version : 100130
 File Encoding         : 65001

 Date: 18/05/2020 12:08:48
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for inject_store
-- ----------------------------
DROP TABLE IF EXISTS `inject_store`;
CREATE TABLE `inject_store`  (
  `id` int(255) NOT NULL AUTO_INCREMENT,
  `ip_address` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL DEFAULT NULL,
  `hwid` varbinary(255) NULL DEFAULT NULL,
  `pattern` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `created_at` datetime(6) NULL DEFAULT NULL,
  PRIMARY KEY (`id`, `pattern`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 67 CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Compact;

SET FOREIGN_KEY_CHECKS = 1;
